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
#include "mads/madsv2/core/sprite.h"
#include "mads/madsv2/core/buffer.h"
#include "mads/madsv2/core/mouse.h"
#include "mads/madsv2/core/font.h"
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/core/sort.h"
#include "mads/madsv2/core/mem.h"
#include "mads/madsv2/core/pal.h"
#include "mads/madsv2/core/magic.h"
#include "mads/madsv2/core/mcga.h"
#include "mads/madsv2/core/tile.h"
#include "mads/madsv2/core/ems.h"
#include "mads/madsv2/core/keys.h"
#include "mads/madsv2/core/timer.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/engine.h"

#define word_align_mattes
#ifndef disable_error_check
#include "mads/madsv2/core/error.h"
#endif

#include "mads/madsv2/core/video.h"
#include "mads/madsv2/core/anim.h"
#include "mads/madsv2/core/matte.h"

#ifdef show_mattes
#include "mads/madsv2/core/screen.h"
#include "mads/madsv2/core/keys.h"
#endif

namespace MADS {
namespace MADSV2 {

/* Global data structures */

SeriesPtr series_list[SERIES_LIST_SIZE + SERIES_BONUS_SIZE];
char      series_name[SERIES_LIST_SIZE][9];
byte      series_user[SERIES_LIST_SIZE];

Image image_list [IMAGE_LIST_SIZE];
Matte matte_list [MATTE_LIST_SIZE];

byte depth_list_id[MATTE_LIST_SIZE];
word depth_list   [MATTE_LIST_SIZE];

Message message_list [MESSAGE_LIST_SIZE];

int series_list_marker = 0;

int picture_view_x = 0;         /* Absolute picture view coordinates */
int picture_view_y = 0;

int viewing_at_x       = 0;     /* Viewport alignment from top/left screen  */
int viewing_at_y       = 0;

int matte_disable_screen_update = false;

byte attr_packed    = false;

byte image_marker       = 0;

Buffer scr_main  = { 0, 0, NULL };      /* Full screen work buffer   */
Buffer scr_work  = { 0, 0, NULL };      /* Room art work buffer      */

Buffer scr_orig  = { 0, 0, NULL };      /* Room original art buffer  */
Buffer scr_depth = { 0, 0, NULL };      /* Depth code buffer         */
Buffer scr_walk  = { 0, 0, NULL };      /* Walk code buffer          */
Buffer scr_special={ 0, 0, NULL };      /* Special code buffer       */

int matte_guard_depth_0 = false;        /* Guard popup window depth  */

int work_screen_ems_handle = -1;        /* Work screen in EMS        */

TileMapHeader picture_map,      depth_map;      /* Tile maps         */
TileResource  picture_resource, depth_resource; /* Tile resources    */

ImageInter image_inter_list[IMAGE_INTER_LIST_SIZE];
Matte      matte_inter_list[IMAGE_INTER_LIST_SIZE];

int inter_viewing_at_y = 0;             /* Interface base Y loc.     */

byte image_inter_marker = 0;            /* Interface image list mark */

Buffer scr_inter = { 0, 0, NULL }; /* Interface work buffer     */
Buffer scr_inter_orig = { 0, 0, NULL }; /* Interface original buffer */


int matte_map_work_screen() {
	int error_flag = false;

	if (work_screen_ems_handle >= 0) {
		if (ems_mapping_changed) {
			error_flag = ems_map_buffer(work_screen_ems_handle);
		}
		ems_mapping_changed = false;
	}

	return error_flag;
}

static void matte_init_series() {
	int count;

	series_list_marker = 0;

	for (count = 0; count < SERIES_LIST_SIZE; count++) {
		series_list[count] = NULL;
		series_name[count][0] = 0;
		series_user[count] = 0;
	}
}

int matte_allocate_series(SeriesPtr series, int bonus_series_number) {
	int handle;

	handle = -1;
	if (bonus_series_number) {
		handle = SERIES_LIST_SIZE + bonus_series_number - 1;
		series_list[handle] = series;
	} else {
		if (series_list_marker < SERIES_LIST_SIZE) {
			handle = series_list_marker++;
			series_list[handle] = series;
		}
	}
#ifndef disable_error_check
	if (handle < 0) {
		error_report(ERROR_SERIES_LIST_FULL, ERROR, MODULE_MATTE, SERIES_LIST_SIZE, 0);
	}
#endif

	return handle;
}

int matte_load_series(const char *name, int load_flags, int bonus_series_number) {
	int found = false;
	int valid_name = true;
	int handle = -1;
	int count;
	char name_buf[15];
	char *mark;
	SeriesPtr series;


	strncpy(name_buf, name, 14);
	name_buf[13] = 0;
	if (name_buf[0] == '*') {
		Common::strcpy_s(name_buf, &name_buf[1]);
	}
	mark = strchr(name_buf, '.');
	if (mark != NULL) *mark = 0;

	if ((strlen(name) > 13) || (strlen(name_buf) > 8)) {
		Common::strcpy_s(name_buf, "%%");
		valid_name = false;
	}
	mads_strupr(name_buf);

	if (valid_name && !bonus_series_number) {
		for (count = 0; !found && (count < SERIES_LIST_SIZE); count++) {
			if (series_user[count]) {
				if (!strcmp(series_name[count], name_buf)) {
					found = true;
					handle = count;
					series_user[count]++;
				}
			}
		}
	}

	if (!found) {

		series = sprite_series_load(name, load_flags);
		if (series == NULL) {
			goto done;
		}

		handle = matte_allocate_series(series, bonus_series_number);
		if (handle < 0) {
			goto done;
		}

		if (handle < SERIES_LIST_SIZE) {
			Common::strcpy_s(series_name[handle], name_buf);
			series_user[handle] = 1;
		}
	}

done:
	return handle;
}

void matte_deallocate_series(int id, int free_memory) {
	if (id < 0) goto done;

	if (id < SERIES_LIST_SIZE) {
		if (series_user[id]) {
			series_user[id]--;
			if (series_user[id]) goto done;
		}
	}

	sprite_free(&series_list[id], free_memory);
	series_list[id] = NULL;

	// Protect against memory fragmentation
	if (id < SERIES_LIST_SIZE) {
		if (id == series_list_marker - 1) {
			series_list_marker--;
		} else {
#ifndef disable_error_check
			error_report(ERROR_WRONG_SERIES_UNLOAD_ORDER, WARNING, MODULE_MATTE, id, series_list_marker);
#endif
		}
	}

done:
	;
}

static void matte_init_messages() {
	int count;
	for (count = 0; count < MESSAGE_LIST_SIZE; count++) {
		message_list[count].active = false;
	}
}

void matte_init(int init_series) {
	matte_init_messages();
	if (init_series) matte_init_series();

	image_marker = 1;
	image_list[0].flags = IMAGE_REFRESH;
	image_list[0].segment_id = (byte)-1;
}

int matte_allocate_image() {
	int result;

	if (image_marker >= IMAGE_LIST_SIZE) {
#if !defined(disable_error_check)
		error_report(ERROR_IMAGE_LIST_FULL, ERROR, MODULE_MATTE, IMAGE_LIST_SIZE, image_marker);
#endif
		result = -1;
	} else {
		result = image_marker++;
	}

	return result;
}

void matte_refresh_work() {
	int id;

	id = matte_allocate_image();
	image_list[id].flags = IMAGE_REFRESH;
	image_list[id].segment_id = (byte)-1;
}

int matte_add_message(FontPtr font, char *text, int x, int y, int message_color, int auto_spacing) {
	int message_handle;
	int count;

	message_handle = -1;
	for (count = 0; (count < MESSAGE_LIST_SIZE) && (message_handle < 0); count++) {
		if (!message_list[count].active) {
			message_handle = count;
			message_list[message_handle].x = x;
			message_list[message_handle].y = y;
			message_list[message_handle].font = font;
			message_list[message_handle].text = text;
			message_list[message_handle].xs = font_string_width(font, text, auto_spacing);
			message_list[message_handle].ys = font->max_y_size;
			message_list[message_handle].main_color = message_color;
			message_list[message_handle].spacing = (char)auto_spacing;
			message_list[message_handle].status = 1;
			message_list[message_handle].active = true;
		}
	}
#ifndef disable_error_check
#ifndef disable_minor_error
	if (message_handle < 0) {
		error_report(ERROR_MESSAGE_LIST_FULL, ERROR, MODULE_MATTE, MESSAGE_LIST_SIZE, 0);
	}
#endif
#endif
	return message_handle;
}

void matte_clear_message(int handle) {
	message_list[handle].status = -1;
}

void bound_matte(MattePtr matte, int xs, int ys, int maxx, int maxy) {
	int x2, y2;

#if defined(word_align_mattes)
	if (matte->x & 1) {
		matte->x -= 1;
		xs++;
	}
	if (xs & 1) {
		xs++;
	}
#endif

	x2 = matte->x + xs - 1;  // Determine right most point
	matte->x = MAX(0, matte->x);  // Scale coordinates to work
	x2 = MAX(0, x2);
	x2 = MIN((maxx - 1), x2);
	matte->xs = (x2 - matte->x) + 1;  // Determine horizontal size

	matte->xh = matte->xs >> 1;  // Set half-width
	matte->xc = matte->x + (((matte->xs + 1) >> 1) - 1);  // Set center mark

	y2 = matte->y + ys - 1;  // Determine lower most point
	matte->y = MAX(0, matte->y);  // Scale coordinates to work
	y2 = MAX(0, y2);
	y2 = MIN((maxy - 1), y2);
	matte->ys = (y2 - matte->y) + 1;  // Determine vertical size

	matte->yh = matte->ys >> 1;  // Set half-height
	matte->yc = matte->y + (((matte->ys + 1) >> 1) - 1);  // Set center mark

	matte->valid = true;
	matte->linked_matte = nullptr;
}

/**
 * Proposes a matte-ing scheme (matte) for the specified image.  This
 * initial matte is the smallest possible rectangle which will completely
 * enclose its sprite.
 */
static void make_matte(ImagePtr image, MattePtr matte) {
	SpritePtr sprite;
	int xs, ys;

	if (image->flags == IMAGE_REFRESH) {
		matte->x = 0;
		matte->y = 0;
		xs = scr_work.x;
		ys = scr_work.y;
	} else {

		matte->x = image->x - picture_map.pan_x;
		matte->y = image->y - picture_map.pan_y;

		sprite = &series_list[image->series_id]->index[(image->sprite_id & SPRITE_MASK) - 1];

		if (image->scale == IMAGE_UNSCALED) {
			xs = sprite->xs;
			ys = sprite->ys;
		} else {
			xs = (50 + (sprite->xs * image->scale)) / 100;
			ys = (50 + (sprite->ys * image->scale)) / 100;
			matte->x -= (xs >> 1);
			matte->y -= (ys - 1);
		}
	}

	bound_matte(matte, xs, ys, scr_work.x, scr_work.y);
}

static void make_message_matte(int handle, MattePtr matte) {
	matte->x = message_list[handle].x;
	matte->y = message_list[handle].y;
	bound_matte(matte, message_list[handle].xs, message_list[handle].ys,
		scr_work.x, scr_work.y);
}

/**
 * matte1, matte2  = pointers to mattes to be checked for
 * collisions (i.e. overlapping areas).
 * Returns TRUE if the two specified mattes overlap and need to be
 * combined into one big matte.
 * (Algorithm: overlap if on each axis the distance between the centers
 * of the mattes is less-than/equal-to the sum of the half widths)
 */
static int check_collisions(MattePtr matte1, MattePtr matte2) {
	return (ABS(matte1->xc - matte2->xc) <= (matte1->xh + matte2->xh)) &&
		(ABS(matte1->yc - matte2->yc) <= (matte1->yh + matte2->yh));
}

/**
 * matte1, matte2  = pointers to the mattes to be merged (matte1
 * will receive the combined matte).
 */
static void combine_mattes(MattePtr matte1, MattePtr matte2) {
	int highx, highy;

	// Get the maximum x and y values (+1) of the two mattes
	highx = MAX((matte1->x + matte1->xs), (matte2->x + matte2->xs));
	highy = MAX((matte1->y + matte1->ys), (matte2->y + matte2->ys));

	// Get the minimum x and y values of the two mattes; these will
	// serve as the x/y base position of the combined matte.
	matte1->x = MIN(matte1->x, matte2->x);
	matte1->y = MIN(matte1->y, matte2->y);

	// Subtract the minimums from the maximums+1 to get the new matte's
	// size.  Then, compute the half heights/widths and the center points.
	// Note that since we are doing integer "division", we must adjust to
	// make sure that lost bits do not result in a failed collision test.
	matte1->xs = highx - matte1->x;
	matte1->xh = matte1->xs >> 1;
	matte1->xc = matte1->x + (((matte1->xs + 1) >> 1) - 1);

	matte1->ys = highy - matte1->y;
	matte1->yh = matte1->ys >> 1;
	matte1->yc = matte1->y + (((matte1->ys + 1) >> 1) - 1);

	// Mark matte2 as EMPTY, but leave behind a pointer to matte1
	matte2->valid = false;
	matte2->linked_matte = matte1;

	// Set matte1's update flag TRUE; we need to redraw everything in
	// this matte.
	matte1->changed = true;
}

void filter_matte_list(MattePtr matte, int size, int base_index) {
	int index1, index2, any_more;
	MattePtr matte1;
	MattePtr matte2;

	// Loop until we make a perfect pass (no collisions) through list
	for (any_more = true; any_more;) {
		any_more = false;

		// Outer index counts through unfiltered mattes only
		matte1 = &matte[base_index] - 1;
		for (index1 = base_index; index1 < size; index1++) {
			matte1++;

			// Ignore empty matte blocks
			if (matte1->valid) {

				// Inner index counts through all mattes with lower indices
				// than our current outer index.
				matte2 = matte - 1;
				for (index2 = 0; index2 < index1; index2++) {
					matte2++;

					// Again, ignore empty matte blocks
					if (matte2->valid) {

						// We've got two real mattes; check for collisions
						if (check_collisions(matte1, matte2)) {

							// Mattes overlap; check if either has changed flag set
							if (matte1->changed || matte2->changed) {

								// Active ("changed") matte triggers collision; combine into
								// one big messy matte.
								combine_mattes(matte1, matte2);

								// Mark that this is no longer a perfect pass; we'll have to
								// loop through the whole thing again.
								any_more = true;

								// And now, for your pleasure, we have no less
								// than EIGHT consecutive close braces!!! What
								// fun! Whoopee! This is C's finest hour!
							}
						}
					}
				}
			}
		}
	}
}

static void matte_quick_to_black(byte *special_pal, int ticks) {
	int going;
	byte *source;
	byte *dest;
	long fade_clock;
	long now_clock;
	byte increments[768];

	source = special_pal;
	dest = increments;

	// Build increment table: each entry is ceil(source[i] / 4), minimum 1
	for (int i = 0; i < 768; i++)
	{
		byte inc = (source[i] >> 2) + ((source[i] & 3) ? 1 : 0);
		if (inc == 0)
			inc = 1;
		dest[i] = inc;
	}

	do {
		going = false;
		fade_clock = timer_read_600() + ticks;

		for (int i = 0; i < 768; i++) {
			byte val = source[i];
			byte decr = dest[i];

			if (val >= decr)
				val -= decr;
			else
				val = 0;

			source[i] = val;

			if (val != 0)
				going = true;
		}

		mcga_setpal((Palette *)special_pal);

		do {
			now_clock = timer_read_600();
		} while (now_clock < fade_clock);

	} while (going);
}

static void matte_quick_from_black(byte *special_pal, int ticks) {
	int going;
	byte *source;
	byte *dest;
	byte *special;
	byte increments[768];
	long fade_clock;
	long now_clock;

	source = &master_palette[0].r;
	special = increments;
	dest = special_pal;

	// Build increment table from master palette: ceil(source[i] / 4), minimum 1
	for (int i = 0; i < 768; i++) {
		byte inc = (source[i] >> 2) + ((source[i] & 3) ? 1 : 0);
		if (inc == 0)
			inc = 1;
		special[i] = inc;
	}

	do {
		going = false;
		fade_clock = timer_read_600() + ticks;

		for (int i = 0; i < 768; i++) {
			byte current = dest[i];  // current fading value (starts at black)
			byte target = source[i];  // master palette ceiling for this channel
			byte inc = special[i];

			if ((int)current + inc >= target)
				current = target;  // clamp to target; channel is done
			else
			{
				current += inc;
				going = true;  // still short of target, keep going
			}

			dest[i] = current;
		}

		mcga_setpal((Palette *)special_pal);

		do {
			now_clock = timer_read_600();
		} while (now_clock < fade_clock);

	} while (going);
}

static void matte_special_effect(int special_effect, int full_screen) {
	int  count;
	int  pixel_rate;
	byte *background_swap;
	Palette special_pal;
	Buffer *work_screen;
	Buffer scr_live = { video_y, video_x, mcga_video };

	work_screen = full_screen ? &scr_main : &scr_work;

	if (viewing_at_y) {
		work_screen = &scr_work;
	}

	mouse_hide();

	if (!timer_low_semaphore) {
		if ((special_effect == MATTE_FX_FADE_FROM_BLACK) ||
			(special_effect == MATTE_FX_FADE_THRU_BLACK)) {
			if (mem_get_avail() < 3600) special_effect = MATTE_FX_FAST_AND_FANCY;
		}

		if (special_effect == MATTE_FX_FAST_AND_FANCY) {
			if (viewing_at_y || (mem_get_avail() < sizeof(Palette))) {
				special_effect = MATTE_FX_FAST_THRU_BLACK;
			}
		}
	}

	switch (special_effect) {
	case MATTE_FX_FADE_FROM_BLACK:
	case MATTE_FX_FADE_THRU_BLACK:
		for (count = 0; count < 3; count++) {
			magic_color_flags[count] = false;
			magic_color_values[count] = 0;
		}

		if (special_effect == MATTE_FX_FADE_THRU_BLACK) {
			mcga_getpal(&special_pal);
			matte_quick_to_black(&special_pal[0].r, 1);
			// magic_fade_to_grey (special_pal, NULL, 0, 256, 0, 1, 1, 16);
			buffer_fill(scr_live, 0);
		}

		memset(special_pal, 0, sizeof(Palette));
		mcga_setpal(&special_pal);

		video_update(work_screen, 0, 0,
			viewing_at_x, viewing_at_y,
			work_screen->x, work_screen->y);

		matte_quick_from_black(&special_pal[0].r, 1);
		// magic_fade_from_grey (special_pal, master_palette, 0, 256, 0, 1, 1, 16);
		break;

	case MATTE_FX_CORNER_LOWER_LEFT:
	case MATTE_FX_CORNER_LOWER_RIGHT:
	case MATTE_FX_CORNER_UPPER_LEFT:
	case MATTE_FX_CORNER_UPPER_RIGHT:
		magic_screen_change_corner(work_screen, master_palette,
			(special_effect - MATTE_FX_CORNER_LOWER_LEFT) + 1,
			0, 0,
			viewing_at_x, viewing_at_y,
			MAGIC_SECRET_FUJI, true, 1);
		break;

	case MATTE_FX_EDGE_LEFT:
	case MATTE_FX_EDGE_RIGHT:
		magic_screen_change_edge(work_screen, master_palette,
			special_effect - MATTE_FX_EDGE_LEFT,
			0, 0,
			viewing_at_x, viewing_at_y,
			MAGIC_SECRET_FUJI, true, 1);
		break;

	case MATTE_FX_CIRCLE_OUT_SLOW:
	case MATTE_FX_CIRCLE_IN_SLOW:
	case MATTE_FX_CIRCLE_OUT_FAST:
	case MATTE_FX_CIRCLE_IN_FAST:
		pixel_rate = 1;
		if (special_effect >= MATTE_FX_CIRCLE_OUT_FAST) {
			special_effect -= 2;
			pixel_rate = 2;
		}
		magic_screen_change_circle(work_screen, master_palette,
			special_effect - MATTE_FX_CIRCLE_OUT_SLOW,
			0, 0,
			viewing_at_x, viewing_at_y,
			MAGIC_SECRET_FUJI, true, 1, pixel_rate);
		break;

	case MATTE_FX_FAST_THRU_BLACK:
		memset(special_pal, 0, sizeof(Palette));
		mcga_setpal(&special_pal);
		buffer_fill(scr_live, 0);
		video_update(work_screen, 0, 0, viewing_at_x, viewing_at_y,
			work_screen->x, work_screen->y);
		mcga_setpal(&master_palette);
		break;

	case MATTE_FX_FAST_AND_FANCY:
	default:
		background_swap = &special_pal[0].r;
		magic_swap_foreground(background_swap, master_palette);
		buffer_rect_translate(*work_screen, scr_live,
			0, 0, viewing_at_x, viewing_at_y,
			work_screen->x, work_screen->y,
			background_swap);
		mcga_setpal(&master_palette);
		video_update(work_screen, 0, 0,
			viewing_at_x, viewing_at_y,
			work_screen->x, work_screen->y);
		break;
	}

	mouse_show();
}

void matte_frame(int special_effect, int full_screen) {
	Matte *matte;
	Image *image;
	int id;
	int id2;
	int x, y;
	int depth_size;
	int beware_the_mouse = false;
	int any_refresh;
	byte new_marker;
	byte high_color, low_color;
	Matte *matte2;
	Image *image2;
	Message *message;
	SpritePtr  sprite;
#ifdef show_mattes
	char temp_buf[80];
	int count;
#endif

	// Make sure work buffer is mapped into the page frame
	matte_map_work_screen();

	any_refresh = false;

	image = image_list;
	matte = matte_list;
	for (id = 0; id < (int)image_marker; id++) {
		if (image->flags < IMAGE_STATIC) {
			matte->changed = true;
			make_matte(image, matte);

			if (image->flags == IMAGE_DELTA) {
				x = image->x;
				y = image->y;
				if (image->scale != IMAGE_UNSCALED) {
					sprite = &series_list[image->series_id]->index[(image->sprite_id & SPRITE_MASK) - 1];
					x -= (sprite->xs >> 1);
					y -= (sprite->ys - 1);
				}

				if ((image->depth <= 1) && !matte_guard_depth_0) {
					sprite_draw(series_list[image->series_id], image->sprite_id,
						&scr_orig,
						x - picture_map.pan_base_x,
						y - picture_map.pan_base_y);
				} else {
					sprite_draw_3d_big(series_list[image->series_id],
						image->sprite_id,
						&scr_orig, &scr_depth,
						x - picture_map.pan_base_x,
						y - picture_map.pan_base_y,
						image->depth, 0, 0);
				}
			}

		} else {
			matte->valid = false;
		}

		if (image->flags == IMAGE_REFRESH) {
			any_refresh = true;
		}

		matte++;
		image++;
	}

	for (id = image_marker; id < FIRST_MESSAGE_MATTE; id++) {
		matte->valid = false;
		matte++;
		// matte_list[id].valid = false;
	}

	message = message_list;
	for (id = 0; id < MESSAGE_LIST_SIZE; id++) {
		// index = id + FIRST_MESSAGE_MATTE;
		if ((message->status < 0) && message->active) {
			matte->changed = true;
			make_message_matte(id, matte);
		} else {
			matte->valid = false;
		}
		message++;
		matte++;
	}

	// Erasures
	if (!any_refresh) {
		filter_matte_list(matte_list, MATTE_LIST_SIZE, 1);

		matte = matte_list;

		for (id = 0; id < MATTE_LIST_SIZE; id++) {

			if (matte->valid) {

				if ((matte->xs > 0) && (matte->ys > 0)) {

					buffer_rect_copy_2(scr_orig, scr_work,
						matte->x + picture_map.pan_offset_x,
						matte->y + picture_map.pan_offset_y,
						matte->x, matte->y,
						matte->xs, matte->ys);

				}
			}

			matte++;
		}

	} else {
		buffer_rect_copy_2(scr_orig, scr_work,
			picture_map.pan_offset_x,
			picture_map.pan_offset_y,
			0, 0,
			video_x, display_y);
	}

	matte = matte_list;
	image = image_list;
	for (id = 0; id < (int)image_marker; id++) {
		if (image->flags >= IMAGE_STATIC) {
			make_matte(image, matte);
			matte->changed = (byte)(image->flags > IMAGE_STATIC);
			image->flags = IMAGE_STATIC;
		}
		matte++;
		image++;
	}

	matte = &matte_list[FIRST_MESSAGE_MATTE];
	message = message_list;
	for (id = 0; id < MESSAGE_LIST_SIZE; id++) {
		if (message->active) {
			if (message->status >= 0) {
				make_message_matte(id, matte);
				matte->changed = (byte)(message->status > 0);
			}
		}
		matte++;
		message++;
	}

	// Check our new matte list for collisions
	if (!any_refresh) filter_matte_list(matte_list, MATTE_LIST_SIZE, 1);

	// Now, create the depth list for our currently active series.  Only
	// create depth list entries for those images which belong to mattes
	// that have their "changed" flags set (i.e. mattes which need to be
	// redrawn this round.
	image = image_list;
	matte = matte_list;
	for (id = depth_size = 0; id < (int)image_marker; id++) {

		if (image->flags >= IMAGE_STATIC) {

			// Search through the matte list to find the matte of which this
			// image is a part.
			for (matte2 = matte; !matte2->valid; matte2 = matte2->linked_matte) {
			}

			// If its matte is being updated, make a depth list entry for
			// our sprite.
			if (matte2->changed || any_refresh) {
				depth_list_id[depth_size] = (byte)id;
				depth_list[depth_size] = 16 - image->depth;
				depth_size++;
			}
		}
		matte++;
		image++;
	}

	// Sort the depth list so that "deeper" sprites will be drawn first
	// and thus appear "behind" the other "nearer" sprites.
	sort_insertion_16(depth_size, depth_list_id, depth_list);

	// Now, run through our depth list, and for each entry, draw the
	// indicated sprite into the work buffer.
	for (id = 0; id < depth_size; id++) {

		// Get the index for the series for this depth list entry
		id2 = depth_list_id[id];

		// Draw the sprite into the work buffer at the appropriate depth
		if (image_list[id2].scale >= 100) {
			if (image_list[id2].scale == IMAGE_UNSCALED) {
				x = image_list[id2].x - picture_map.pan_x;
				y = image_list[id2].y - picture_map.pan_y;
			} else {
				sprite = &series_list[image_list[id2].series_id]->index[(image_list[id2].sprite_id & SPRITE_MASK) - 1];
				x = image_list[id2].x - picture_map.pan_x - (sprite->xs >> 1);
				y = image_list[id2].y - picture_map.pan_y - (sprite->ys - 1);
			}
			if ((image_list[id2].depth <= 1) && !matte_guard_depth_0) {
				sprite_draw(series_list[image_list[id2].series_id],
					image_list[id2].sprite_id,
					&scr_work, x, y);
			} else {
				sprite_draw_3d_big(series_list[image_list[id2].series_id],
					image_list[id2].sprite_id,
					&scr_work, &scr_depth,
					x, y,
					image_list[id2].depth,
					picture_map.pan_offset_x,
					picture_map.pan_offset_y);
			}
		} else {
			sprite_draw_3d_scaled_big(series_list[image_list[id2].series_id],
				image_list[id2].sprite_id,
				&scr_work, &scr_depth,
				image_list[id2].x - picture_map.pan_x,
				image_list[id2].y - picture_map.pan_y,
				image_list[id2].depth,
				image_list[id2].scale,
				picture_map.pan_offset_x,
				picture_map.pan_offset_y);
		}
	}

	// Now draw any messages that need to be updated
	message = message_list;
	matte = &matte_list[FIRST_MESSAGE_MATTE];
	for (id = 0; id < MESSAGE_LIST_SIZE; id++) {
		if (message->active && (message->status >= 0)) {
			for (matte2 = matte; !matte2->valid; matte2 = matte2->linked_matte) {
			}

			if (matte2->changed || any_refresh) {
				high_color = (byte)message->main_color;
				low_color = (byte)(message->main_color >> 8);
				if (!low_color) low_color = high_color;
				font_set_colors(-1,
					high_color,
					low_color,
					0);
				font_write(message->font,
					&scr_work, message->text,
					message->x, message->y, message->spacing);
			}
		}
		message++;
		matte++;
	}

	// Finally, run through our combined matte list, and update any
	// areas of the screen flagged as "changed" by copying from the
	// work screen to the live video screen.
	mouse_set_work_buffer(scr_work.data, scr_work.x);
	mouse_set_view_port_loc(viewing_at_x, viewing_at_y,
		viewing_at_x + scr_work.x - 1,
		viewing_at_y + scr_work.y - 1);

	mouse_freeze();  // Lock out mouse driver

	if ((video_mode != ega_mode) && !special_effect) {
		beware_the_mouse = mouse_refresh_view_port();  // Prepare cursor overlay
	}

	if (!matte_disable_screen_update) {

		if (!special_effect) {

			if (!any_refresh) {

				matte = matte_list;

				for (id = 0; id < MATTE_LIST_SIZE; id++) {

					// Get next matte
#ifdef show_mattes
					sprintf(temp_buf, "(%d, %d) => (%d, %d)   valid: %d   changed: %d      ",
						matte->x, matte->y, matte->xs, matte->ys, matte->valid, matte->changed);
					screen_show(temp_buf, 0, id);
#endif

					// Ignore empty mattes, or images which did not change
					if (matte->valid && matte->changed && (matte->xs > 0) && (matte->ys > 0)) {

						video_update(&scr_work,
							matte->x, matte->y,
							matte->x + viewing_at_x,
							matte->y + viewing_at_y,
							matte->xs, matte->ys);

					}

					matte++;
				}

			} else {
				video_update(&scr_work,
					0, 0,
					viewing_at_x,
					viewing_at_y,
					video_x, display_y);
			}

#ifdef sixteen_colors
			if (video_mode == ega_mode) {
				beware_the_mouse = mouse_refresh_view_port();  // Prepare cursor overlay
				video_flush_ega(viewing_at_y, scr_work.y);  // Update the EGA screen
			}
#endif
		} else {
			matte_special_effect(special_effect, full_screen);
			sound_queue_flush();
		}
	}

#ifdef show_mattes
	keys_get();
#endif

	if (beware_the_mouse) {
		mouse_refresh_done();  // Remove cursor image from work buffer
	}

	mouse_thaw();  // Release the mouse driver

	// Delete erasures from image list
	new_marker = 0;
	image = image_list;
	image2 = image_list;
	for (id = 0; id < (int)image_marker; id++) {
		if (image->flags >= IMAGE_STATIC) {
			if (image != image2) {
				*image2 = *image;
			}
			image2++;
			new_marker++;
		}
		image++;
	}
	image_marker = new_marker;

	// Delete erasures from message list
	message = message_list;
	for (id = 0; id < MESSAGE_LIST_SIZE; id++) {
		if (message->status < 0) {
			message->active = false;
			message->status = 0;
		}
		message++;
	}
}


int matte_allocate_inter_image(void) {
	int result;

	if (image_inter_marker >= IMAGE_INTER_LIST_SIZE) {
#if !defined(disable_error_check)
		error_report(ERROR_IMAGE_INTER_LIST_FULL, ERROR, MODULE_MATTE, IMAGE_INTER_LIST_SIZE, image_inter_marker);
#endif
		result = -1;
	} else {
		result = image_inter_marker++;
	}

	return (result);
}


void matte_refresh_inter(void) {
	int id;

	id = matte_allocate_inter_image();
	image_inter_list[id].flags = IMAGE_REFRESH;
	image_inter_list[id].segment_id = (byte)-1;
}


static void make_inter_matte(ImageInterPtr image, MattePtr matte) {
	SpritePtr sprite;
	int xs, ys;
	int flags;

	flags = image->flags;
	if (flags <= IMAGE_UPDATE_ONLY)  flags -= IMAGE_UPDATE_ONLY;
	if (flags >= IMAGE_UPDATE_READY) flags &= ~IMAGE_UPDATE_READY;

	if (flags == IMAGE_REFRESH) {
		matte->x = 0;
		matte->y = 0;
		xs = scr_inter.x;
		ys = scr_inter.y;
	} else if (flags == IMAGE_OVERPRINT) {
		matte->x = image->x;
		matte->y = image->y;
		xs = image->sprite_id;
		ys = image->series_id;
	} else {

		sprite = &series_list[image->series_id]->index[(image->sprite_id & HALF_SPRITE_MASK) - 1];

		xs = sprite->xs;
		ys = sprite->ys;

		if (image->segment_id == INTER_SPINNING_OBJECT) {
			matte->x = image->x;
			matte->y = image->y;
		} else {
			matte->x = image->x - (xs >> 1);
			matte->y = image->y - (ys - 1);
		}

	}

	bound_matte(matte, xs, ys, scr_inter.x, scr_inter.y);
}



void matte_inter_frame(int update_live, int clear_chaff) {
	int id;
	int x, y;
	int flags;
	word mirror;
	word which;
	SeriesPtr series;
	int beware_the_mouse = false;
	byte new_marker;
	MattePtr matte;
	MattePtr matte2;
	MattePtr i_am_the_dog_master = NULL;
	ImageInter *image;
	ImageInter *image2;
#ifdef show_mattes
	char temp_buf[80];
	int count;
#endif

	// Make sure work buffer is mapped into the page frame
	matte_map_work_screen();

	// Before performing erasures, make a matte for each potential erasure
	// image.
	image = image_inter_list;
	matte = matte_inter_list;
	for (id = 0; id < (int)image_inter_marker; id++) {
		if (image->flags < IMAGE_STATIC) {
			make_inter_matte(image, matte);
			matte->changed = true;
			if (image->segment_id == INTER_SPINNING_OBJECT) {
				if (image->flags == IMAGE_FULLUPDATE) {
					matte->valid = false;
					i_am_the_dog_master = matte;
				}
			}
		} else {
			matte->valid = false;
		}
		image++;
		matte++;
	}

	filter_matte_list(matte_inter_list, image_inter_marker, 1);

	if (i_am_the_dog_master != NULL) i_am_the_dog_master->valid = true;

	// Erasures
	matte = matte_inter_list;
	image = image_inter_list;
	for (id = 0; id < (int)image_inter_marker; id++) {

		if (matte->valid) {

			if ((matte->xs > 0) && (matte->ys > 0)) {

				if (image->flags > IMAGE_UPDATE_ONLY) {
					if (image->flags < IMAGE_ERASE) {
						buffer_rect_copy(scr_inter_orig, scr_inter,
							matte->x, matte->y,
							matte->xs, matte->ys);
					} else {
						buffer_inter_merge_2(scr_inter_orig, scr_inter,
							matte->x, matte->y,
							matte->x, matte->y,
							matte->xs, matte->ys);
					}
				}
			}
		}
		matte++;
		image++;
	}

	matte = matte_inter_list;
	image = image_inter_list;
	for (id = 0; id < (int)image_inter_marker; id++) {
		flags = image->flags;
		if (flags >= IMAGE_STATIC) {
			make_inter_matte(image, matte);
			if (!update_live) flags &= ~IMAGE_UPDATE_READY;
			matte->changed = (byte)(flags > IMAGE_STATIC);
			image->flags &= IMAGE_UPDATE_READY;
		}
		matte++;
		image++;
	}

	// Check our new matte list for collisions
	filter_matte_list(matte_inter_list, (int)image_inter_marker, 1);

	// Now, run through our depth list, and for each entry, draw the
	// indicated sprite into the work buffer.
	image = image_inter_list;
	matte = matte_inter_list;
	for (id = 0; id < (int)image_inter_marker; id++) {

		if ((image->flags >= IMAGE_STATIC) && !(image->flags & IMAGE_UPDATE_READY)) {

			// Search through the matte list to find the matte of which this
			// image is a part.
			for (matte2 = matte; !matte2->valid; matte2 = matte2->linked_matte) {
			}

			if (matte2->changed) {
				series = series_list[image->series_id];
				which = image->sprite_id;
				mirror = (which & HALF_MIRROR_MASK) ? MIRROR_MASK : 0;
				which &= HALF_SPRITE_MASK;
				if (image->segment_id == INTER_SPINNING_OBJECT) {
					sprite_draw(series, which, &scr_inter, image->x, image->y);
				} else {
					x = image->x - (series->index[which - 1].xs >> 1);
					y = image->y - (series->index[which - 1].ys - 1);
					sprite_draw_interface(series,
						which | mirror,
						&scr_inter,
						x, y);
				}
			}
		}
		image++;
		matte++;
	}

	if (update_live) {

		// Finally, run through our combined matte list, and update any
		// areas of the screen flagged as "changed" by copying from the
		// work screen to the live video screen.
		mouse_set_work_buffer(scr_inter.data, scr_inter.x);
		mouse_set_view_port_loc(0, inter_viewing_at_y, 319, inter_viewing_at_y + scr_inter.y - 1);

		mouse_freeze();  // Lock out mouse driver

		if (video_mode != ega_mode) {
			beware_the_mouse = mouse_refresh_view_port();  // Prepare cursor overlay
		}

		matte = matte_inter_list;
		for (id = 0; id < (int)image_inter_marker; id++) {

			// Get next matte
#ifdef show_mattes
			sprintf(temp_buf, "(%d, %d) => (%d, %d)   valid: %d   changed: %d      ",
				matte->x, matte->y, matte->xs, matte->ys, matte->valid, matte->changed);
			screen_show(temp_buf, 0, id);
#endif

			// Ignore empty mattes, or images which did not change
			if (matte->valid && matte->changed && (matte->xs > 0) && (matte->ys > 0)) {

				video_update(&scr_inter,
					matte->x, matte->y,
					matte->x, matte->y + inter_viewing_at_y,
					matte->xs, matte->ys);

			}

			matte++;
		}

#ifdef sixteen_colors
		if (video_mode == ega_mode) {
			beware_the_mouse = mouse_refresh_view_port();  // Prepare cursor overlay
			video_flush_ega(inter_viewing_at_y, scr_inter.y);  // Update the EGA screen
		}
#endif

#ifdef show_mattes
		keys_get();
#endif

		if (beware_the_mouse) {
			mouse_refresh_done();  // Remove cursor image from work buffer
		}

		mouse_thaw();  // Release the mouse driver
	}

	// Delete erasures from image list
	new_marker = 0;
	image = image_inter_list;
	image2 = image_inter_list;
	for (id = 0; id < (int)image_inter_marker; id++) {
		if (image->flags >= IMAGE_STATIC) {
			if (update_live) {
				image->flags &= ~IMAGE_UPDATE_READY;
			} else {
				image->flags |= IMAGE_UPDATE_READY;
			}
			if (id != (int)new_marker) {
				*image2 = *image;
			}
			new_marker++;
			image2++;
		} else if (!update_live && !clear_chaff) {
			if (image->flags > IMAGE_UPDATE_ONLY) {
				image->flags += IMAGE_UPDATE_ONLY;
			}
			new_marker++;
			image2++;
		}
		image++;
	}
	image_inter_marker = new_marker;
}

} // namespace MADSV2
} // namespace MADS
