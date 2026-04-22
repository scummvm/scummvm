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
#include "mads/madsv2/core/buffer.h"
#include "mads/madsv2/core/cycle.h"
#include "mads/madsv2/core/ems.h"
#include "mads/madsv2/core/env.h"
#include "mads/madsv2/core/error.h"
#include "mads/madsv2/core/fileio.h"
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/keys.h"
#include "mads/madsv2/core/magic.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/mcga.h"
#include "mads/madsv2/core/mem.h"
#include "mads/madsv2/core/mouse.h"
#include "mads/madsv2/core/object.h"
#include "mads/madsv2/core/pack.h"
#include "mads/madsv2/core/pal.h"
#include "mads/madsv2/core/popup.h"
#include "mads/madsv2/core/room.h"
#include "mads/madsv2/core/sprite.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/core/tile.h"
#include "mads/madsv2/core/video.h"

namespace MADS {
namespace MADSV2 {

#define object_extra_colors     10

ObjectPtr object = NULL;
int       num_objects = 0;

int object_ems_handle = BUFFER_PRESERVE;
char object_speech_resource[20] = "*SPCHNOTE.DSR";


void ObjectBuf::load(Common::SeekableReadStream *src) {
	Common::Serializer s(src, nullptr);
	synchronize(s);
}

void ObjectBuf::synchronize(Common::Serializer &s) {
	s.syncAsUint16LE(vocab_id);
	s.syncAsSint16LE(location);
	s.syncAsByte(prep);
	s.syncAsByte(num_verbs);
	s.syncAsByte(num_qualities);
	s.syncAsByte(syntax);

	for (int i = 0; i < OBJECT_MAX_VERBS; ++i)
		verb[i].synchronize(s);
	s.syncBytes(quality_id, OBJECT_MAX_QUALITIES);
	for (int i = 0; i < OBJECT_MAX_QUALITIES; ++i)
		s.syncAsSint32LE(quality_value[i]);
}


void object_unload(void) {
	if (object != NULL) {
		mem_free(object);
		object = NULL;
	}
}

int object_load(void) {
	int count;
	int error_flag = true;
	long mem_to_get;
	Common::SeekableReadStream *handle;

	handle = env_open("*OBJECTS.DAT", "rb");
	if (handle == NULL) goto done;

	num_objects = handle->readUint16LE();

	mem_to_get = sizeof(Object) * num_objects;
	object = (ObjectPtr)mem_get_name(mem_to_get, "$objects");
	if (object == NULL) goto done;

	for (count = 0; (count < num_objects); count++)
		object[count].load(handle);

	inven_num_objects = 0;
	for (count = 0; count < num_objects; count++) {
		if (object[count].location == PLAYER) {
			if (inven_num_objects < INVEN_MAX_OBJECTS) {
				inven[inven_num_objects++] = count;
			} else {
				error_report(ERROR_PLAYER_INVENTORY_FULL, ERROR, MODULE_OBJECT, inven_num_objects, count);
			}
		}
	}

	error_flag = false;

done:
	delete handle;
	if (error_flag)
		object_unload();

	return error_flag;
}

int object_named(int vocab_id) {
	int named = -1;
	int count;

	for (count = 0; (named < 0) && (count < num_objects); count++) {
		if (vocab_id == (int)object[count].vocab_id) {
			named = count;
		}
	}

	return named;
}

int object_is_here(int object_id) {
	int here_flag = false;

	if (object_id < 0) goto done;

	if (object[object_id].location != room_id) goto done;

	here_flag = true;

done:
	return here_flag;
}

int object_has_quality(int object_id, int quality_id) {
	int has_quality = false;
	int count;

	for (count = 0; count < (int)object[object_id].num_qualities; count++) {
		if (object[object_id].quality_id[count] == (byte)quality_id) {
			has_quality = true;
		}
	}

	return has_quality;
}

long object_check_quality(int object_id, int quality_id) {
	long quality_value = 0;
	int count;

	for (count = 0; count < (int)object[object_id].num_qualities; count++) {
		if (object[object_id].quality_id[count] == (byte)quality_id) {
			quality_value = object[object_id].quality_value[count];
		}
	}

	return quality_value;
}

void object_set_quality(int object_id, int quality_id, long quality_value) {
	int count;

	for (count = 0; count < (int)object[object_id].num_qualities; count++) {
		if (object[object_id].quality_id[count] == (byte)quality_id) {
			object[object_id].quality_value[count] = quality_value;
		}
	}
}

/**
 * Given a buffer and a pointer to a 256 byte map, translates every
 * color in the buffer using the map.
 */
static void object_remap_buffer(Buffer *buffer, byte *map) {
	byte *data = buffer->data;
	int   size = buffer->x * buffer->y;

	for (int i = 0; i < size; i++)
		data[i] = map[data[i]];
}

int object_examine(int number, long message, int speech) {
	int restored_screen = false;
	char sprite_name[80];
	byte *old_master_palette;
	byte *greyed_master_palette;
	dword *old_color_status;
	int old_flag_used[PAL_MAXFLAGS];
	int num_colors;
	int cycling_save;
	int y_base;
	//int x_size;// , y_size;
	int count;
	int refresh_flag;
	int object_preserve_handle;
	byte map[256];
	//SeriesPtr object_series = NULL;
	RGBcolor top_eight[8];

	// Wait cursor
	cursor_id = 2;
	if (cursor_id != cursor_last) {
		mouse_cursor_sprite(cursor, cursor_id);
		cursor_last = cursor_id;
	}

	inter_turn_off_object();
	inter_screen_update();

	memcpy(top_eight, &master_palette[248].r, 8 * sizeof(RGBcolor));

	// Use attribute buffer to cheat on memory requirements a bit
	old_master_palette = scr_depth.data;
	greyed_master_palette = scr_depth.data + sizeof(Palette);
	old_color_status = (dword *) (greyed_master_palette + sizeof(Palette));

	sprite_force_memory = ((byte *)old_color_status) + (sizeof(dword) * 256);
	sprite_force_size = 22400;  // Use rest of attribute buffer, at most

	// Notify "magic" that we intend to do a grey scale on all guns
	for (count = 0; count < 3; count++) {
		magic_color_flags[count] = true;
	}

	// Turn off all cycling
	cycling_save = cycling_active;
	cycling_active = false;

	// Get sprite series name
	Common::strcpy_s(sprite_name, "*OB");
	env_catint(sprite_name, number, 3);
	Common::strcat_s(sprite_name, ".SS");

	// Prepare for flicker-free mouse updates
	mouse_set_work_buffer(scr_main.data, video_x);
	mouse_set_view_port_loc(0, 0, video_x - 1, video_y - 1);

	// Save a copy of our work buffer somewhere (probably in EMS)
	matte_map_work_screen();
	object_preserve_handle = buffer_preserve(&scr_main, object_ems_handle, work_screen_ems_handle,
		0, 0, video_x, video_y);
	matte_map_work_screen();

	// Save a copy of current palette structure
	memcpy(old_master_palette, master_palette, sizeof(Palette));
	memcpy(old_color_status, color_status, sizeof(dword) << 8);
	memcpy(old_flag_used, flag_used, sizeof(int) * PAL_MAXFLAGS);

	// Clear out all non-reserved colors in the palette
	num_colors = 256 - (KERNEL_RESERVED_LOW_COLORS + KERNEL_RESERVED_HIGH_COLORS);

	for (count = 0; count < 256; count++) {
		if ((count < KERNEL_RESERVED_LOW_COLORS) || (count >= (256 - (KERNEL_RESERVED_HIGH_COLORS + object_extra_colors)))) {
			map[count] = (byte)count;
			color_status[count] = 1;
		} else {
			color_status[count] = 0;
		}
	}

	for (count = 0; count < PAL_MAXFLAGS; count++) {
		flag_used[count] = 0;
	}

	// Fade to a grey-scale picture
	magic_fade_to_grey(master_palette, &map[KERNEL_RESERVED_LOW_COLORS],
		KERNEL_RESERVED_LOW_COLORS, num_colors,
		OBJECT_GREY_BASE, OBJECT_GREY_COLORS,
		OBJECT_GREY_SPEED, OBJECT_GREY_STEPS);

	// Need to save a copy of greyed out palette
	memcpy(greyed_master_palette, master_palette, sizeof(Palette));

	// Remap the contents of the work screen to the top 8 colors of the
	// palette (the true grey scale to which all other colors have been
	// faded).  This will allow us to change palettes to load the object
	// sprite.
	object_remap_buffer(&scr_main, map);

	// Copy the remapped version of the work buffer onto the screen; this
	// will have no visible effect but will free up a good portion of the
	// palette.
	mouse_freeze();
	refresh_flag = mouse_refresh_view_port();

	video_update(&scr_main, 0, 0, 0, 0, video_x, video_y);

	if (refresh_flag) mouse_refresh_done();
	mouse_thaw();

	mcga_setpal(&master_palette);

	// Load the object series
	// object_series = sprite_series_load (sprite_name, PAL_MAP_BACKGROUND);
	matte_map_work_screen();

	// Now set the palette to include the colors for this series.
	// mcga_setpal (&master_palette);
	y_base = OBJECT_VIEW_OFFSET;

	// Draw the object sprite on the screen
	// if (object_series != NULL) {
	// x_size = object_series->index[0].xs;
	// y_size = object_series->index[0].ys;
	// x_base = (video_x >> 1) - (x_size >> 1);
	// sprite_draw  (object_series, 1, &scr_main, x_base, y_base);
	//
	// mouse_hide();
	//
	// video_update (&scr_main, x_base, y_base, x_base, y_base, x_size, y_size);
	//
	// mouse_show();
	//
	// y_base += y_size;
	// }
	y_base += OBJECT_VIEW_OFFSET;

	if (message) {
		text_saves_screen = false;
		// text_default_y    = y_base;
		// for (count = 0; count < (popup_num_colors - 1); count++) {
		// popup_colors[count] -= object_extra_colors;
		// }
		memcpy(&cycling_palette[248].r, &master_palette[248].r, 8 * sizeof(RGBcolor));

		// pl    if (speech) {
		// if (speech_system_active && speech_on) {
		// speech_play (object_speech_resource, speech);
		// }
		// }
		text_show(message);

		// pl    if (speech && speech_system_active && speech_on) {
		// speech_all_off();
		// }
			// for (count = 0; count < (popup_num_colors - 1); count++) {
			// popup_colors[count] += object_extra_colors;
			// }
		text_saves_screen = true;
		// text_default_y    = POPUP_CENTER;
	} else {
		keys_get();
	}

	// Flush object from memory
	// if (object_series != (SeriesPtr) sprite_force_memory) mem_free (object_series);
	// If we saved the work buffer in EMS, we can now fade back to the
	// original screen.
	if (object_preserve_handle != BUFFER_NOT_PRESERVED) {

		// Restore a copy of the work screen
		matte_map_work_screen();

		buffer_restore_keep_flag = true;
		buffer_restore(&scr_main, object_preserve_handle, work_screen_ems_handle, 0, 0, video_x, video_y);
		buffer_restore_keep_flag = false;

		matte_map_work_screen();

		// We must remap to the special grey version first, so that we can
		// change back to the first palette.
		object_remap_buffer(&scr_main, map);

		// Restore old master palette structure.
		memcpy(master_palette, old_master_palette, sizeof(Palette));
		memcpy(color_status, old_color_status, sizeof(dword) << 8);
		memcpy(flag_used, old_flag_used, sizeof(int) * PAL_MAXFLAGS);

		// Copy a greyed-out version of picture onto the screen (erasing the
		// object picture).
		mouse_freeze();
		refresh_flag = mouse_refresh_view_port();

		video_update(&scr_main, 0, 0, 0, 0, video_x, video_y);

		if (refresh_flag) mouse_refresh_done();
		mouse_thaw();

		// Now we can change back to the fully greyed-out version of the
		// original master palette.
		mcga_setpal((Palette *)greyed_master_palette);

		// We must copy the work buffer from EMS again, since we have destroyed
		// the first version we brought back (by remapping to grey again).
		matte_map_work_screen();

		buffer_restore(&scr_main, object_preserve_handle, work_screen_ems_handle, 0, 0, video_x, video_y);

		matte_map_work_screen();

		// We are ready to copy the true work buffer image back onto the screen.
		// Since the palette is now fully greyed out, there will be no visible
		// change.
		mouse_freeze();
		refresh_flag = mouse_refresh_view_port();

		video_update(&scr_main, 0, 0, 0, 0, video_x, video_y);

		if (refresh_flag) mouse_refresh_done();
		mouse_thaw();

		// Finally, we can fade back to our original palette.
		magic_fade_from_grey((RGBcolor *)greyed_master_palette, master_palette,
			KERNEL_RESERVED_LOW_COLORS, num_colors,
			OBJECT_GREY_BASE, OBJECT_GREY_COLORS,
			OBJECT_GREY_SPEED, OBJECT_GREY_STEPS);

		restored_screen = true;
	}

	// done:
	  // Turn color cycling back on.
	memcpy(&cycling_palette[248].r, top_eight, 8 * sizeof(RGBcolor));
	mcga_setpal_range(&cycling_palette, 248, 8);

	cycling_active = cycling_save;

	// Don't forget to reload the attribute screen which we wrote all
	// over.
	sprite_force_memory = NULL;


	if (room_load_variant(room_id, room_variant, NULL, room,
		&scr_depth, &scr_walk, &scr_special,
		&depth_map, &depth_resource, -1)) {
		error_report(ERROR_VARIANT_LOAD_FAILURE, WARNING, MODULE_OBJECT, room_load_error, (room_id * 10) + room_variant);
	}
	tile_pan(&depth_map, picture_view_x, picture_view_y);

	kernel_force_refresh();

	inter_spin_object(inven[active_inven]);

	return (restored_screen);
}

} // namespace MADSV2
} // namespace MADS
