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
#include "mads/madsv2/core/config.h"
#include "mads/madsv2/core/error.h"
#include "mads/madsv2/core/font.h"
#include "mads/madsv2/core/hspot.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/keys.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/mem.h"
#include "mads/madsv2/core/mouse.h"
#include "mads/madsv2/core/pal.h"
#include "mads/madsv2/core/room.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/sprite.h"
#include "mads/madsv2/core/timer.h"
#include "mads/madsv2/nebular/main_menu.h"
#include "mads/madsv2/nebular/extra.h"
#include "mads/madsv2/nebular/mads/sounds.h"
#include "mads/madsv2/engine.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {

#define CONFIG_FILE_NAME "CONFIG.REX"

#define COMMAND_LINE_MAX        10

#define FRAME_RATE              1
#define MENU_FRAME_RATE         3

#define NUM_MENU_ITEMS          6

#define MENU_APPEARING          0
#define MENU_ACCEPTING_COMMANDS 1
#define MENU_DISAPPEARING       2

#define MENU_HIGH_SPRITE        15

static const byte ARROW_CURSOR[16][16] = {
	{ 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
	{ 0x00, 0x02, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
	{ 0x00, 0x02, 0x02, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
	{ 0x00, 0x02, 0x03, 0x02, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
	{ 0x00, 0x02, 0x03, 0x03, 0x02, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
	{ 0x00, 0x02, 0x03, 0x03, 0x03, 0x02, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
	{ 0x00, 0x02, 0x03, 0x03, 0x03, 0x03, 0x02, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
	{ 0x00, 0x02, 0x03, 0x03, 0x03, 0x03, 0x03, 0x02, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
	{ 0x00, 0x02, 0x03, 0x03, 0x03, 0x03, 0x02, 0x02, 0x02, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
	{ 0x00, 0x02, 0x03, 0x02, 0x03, 0x03, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
	{ 0x00, 0x02, 0x02, 0x00, 0x02, 0x03, 0x02, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
	{ 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x02, 0x03, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
	{ 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x02, 0x03, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
	{ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
	{ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
	{ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }
};

int mads_mode = false;

FontPtr font = NULL;
int font_auto_spacing = -1;

bool new_background = false;
int room_number = 0;

int win_status = 0;

bool force_opening = false;
bool dont_return = false;
bool white_bars = true;
byte white_border = 2;

char command_line[COMMAND_LINE_MAX][80];
char *command_list[COMMAND_LINE_MAX + 1];
int  command_line_size = 0;

int  use_mouse_cursor_fix = false;

bool going;
int  menu_mode;
int  must_perform_matte;

long frame_clock;
long now_clock;
long menu_clock;

int menu_state;
int menu_appear_state;

int highest_y = -1;
int next_x = -1;

char sound_file[80] = "*#SOUND.PH9";
char sound_letter = 'N';

int  sound_board_address = 0x220;              /* Default sound board address */
int  sound_board_type = sound_board_roland; /* Default sound board type    */

int  swapping = false;                   /* Flag when swapping in new background */
int  swap_line;                          /* Currently active swap line           */
long swap_clock;                         /* Time for next swap */

int  current_item;
int  current_eye = false;
int  eye_latch = false;
int  selected_item = -1;
int  animating_item = -1;

#define LEFT_EYE   0
#define RIGHT_EYE  1

Palette special_pal;                     /* Palette for fadeout */


MenuItem menu_item[NUM_MENU_ITEMS + 1];    /* Menu item array */


static void load_title_screen() {
	constexpr int ROOM_ID = 990;
	new_background = true;
	buffer_free(&scr_depth);
	buffer_free(&scr_orig);

	if (room)
		mem_free(room);

	pal_init(4, 8);
	pal_white(master_palette);

	// Set up mouse cursor
	Graphics::Surface surf;
	surf.format = Graphics::PixelFormat::createFormatCLUT8();
	surf.w = surf.pitch = surf.h = 16;
	surf.setPixels(const_cast<byte *>(&ARROW_CURSOR[0][0]));
	mouse_cursor_surface(surf, 1, 1);
	mouse_show();

	// Load the title screen room
	room = RexNebular::room_load_rex(ROOM_ID, 0, nullptr, &scr_orig, &scr_depth, nullptr, nullptr, nullptr,
		nullptr, nullptr, nullptr, 0, 0, 0);
	if (!room)
		error("Could not load title screen room %d", ROOM_ID);

	image_marker = 1;
	image_list[0].flags = IMAGE_REFRESH;
	image_list[0].segment_id = 0xff;
}

static void start_series() {
	int error_flag = true;
	char temp_buf[80];
	SeriesPtr series;
	int count;
	int handle;

	for (count = 0; count <= NUM_MENU_ITEMS; count++) {
		handle = -1;
		Common::strcpy_s(temp_buf, "*RM990A0.SS");
		temp_buf[7] = '1' + count;

		series = sprite_series_load(temp_buf, 0);
		if (!series) goto done;
		handle = matte_allocate_series(series, 0);
		if (handle < 0) goto done;

		menu_item[count].handle = handle;
		menu_item[count].active = true;
		menu_item[count].status = 0;
	}

	menu_item[4].active = config_file.quotes_enabled;
	error_flag = false;

done:
	if (error_flag) {
		Common::strcpy_s(error_string, temp_buf);
		error_report(ERROR_SERIES_LOAD_FAILED, WARNING, MODULE_UNKNOWN, count, handle);
	}
}

static void stop_series() {
	int count;

	for (count = NUM_MENU_ITEMS; count >= 0; count--) {
		matte_deallocate_series(menu_item[count].handle, true);
	}
}

static void start_hotspots() {
	int count;
	int x1, x2, y1, y2;
	int xs, ys;
	SeriesPtr series;

	numspots = 0;

	for (count = 0; count < NUM_MENU_ITEMS; count++) {
		series = series_list[menu_item[count].handle];
		xs = series->index[0].xs;
		ys = series->index[0].ys;
		x1 = series->index[0].x - (xs >> 1);
		y1 = series->index[0].y - (ys - 1);

		x2 = x1 + xs - 1;
		y2 = y1 + ys - 1;
		hspot_add(x1, y1, x2, y2, 1, count, mcga_mode);
	}
}

static void process_menu() {
	int myspot;

	myspot = hspot_which(mouse_x, mouse_y - viewing_at_y, mcga_mode);

	//current_eye = false;

	if ((myspot > 0) && mouse_any_stroke && (menu_mode == MENU_ACCEPTING_COMMANDS)) {
		current_item = spot[myspot].num;
	} else {
		current_item = -1;
	}

	if (mouse_stop_stroke && (current_item >= 0) && (menu_mode == MENU_ACCEPTING_COMMANDS)) {
		selected_item = current_item;
		menu_mode = MENU_DISAPPEARING;
		menu_state = 1;
		sound_queue(N_AllFade);
	}
}

static void process_sprites() {
	int count;
	int sprite;
	int series = 0;
	int look, match;
	Image image;

	for (count = 0; count < (int)image_marker; count++) {
		if (image_list[count].flags >= IMAGE_STATIC) {
			image_list[count].flags = IMAGE_ERASE;
		}
	}

	for (count = 0; count < NUM_MENU_ITEMS; count++) {
		if (menu_item[count].active) {
			image.flags = IMAGE_UPDATE;
			image.segment_id = (byte)(count + 1);

			switch (menu_mode) {
			case MENU_APPEARING:
				series = count;

				if (series < animating_item) {
					sprite = 1;
				} else if (series == animating_item) {
					sprite = menu_state;
				} else {
					sprite = MENU_HIGH_SPRITE + 1;
				}
				break;

			case MENU_ACCEPTING_COMMANDS:
				if (count != current_item) {
					sprite = 1;
					series = count;
				} else {
					sprite = count + 1;
					series = 6;
				}
				break;

			default:
				if (count == selected_item)
					sprite = 1;
				else
					sprite = menu_state;
				series = count;
				break;
			}

			image.series_id = (byte)menu_item[series].handle;
			image.sprite_id = sprite;

			image.x = series_list[series]->index[sprite - 1].x;
			image.y = series_list[series]->index[sprite - 1].y;

			image.depth = 0;
			image.scale = 100;

			match = !(sprite <= MENU_HIGH_SPRITE);

			for (look = 0; !match && (look < (int)image_marker); look++) {
				if (image_list[look].segment_id == image.segment_id) {
					if (image_list[look].equals(image)) {
						image_list[look].flags = IMAGE_STATIC;
						match = true;
					}
				}
			}

			if (!match) {
				image_list[image_marker] = image;
				image_marker++;
			}
		}
	}
}

void menu_control() {
	int fx;
	int mykey;

	menu_mode = MENU_APPEARING;
	animating_item = 0;
	menu_state = MENU_HIGH_SPRITE;
	menu_appear_state = 0;
	menu_clock = 999999999;
	frame_clock = 0;

	going = true;
	must_perform_matte = false;

	frame_clock = 0;

	g_engine->_soundManager->init(7);
	sound_queue(N_TitleScreen);

	load_title_screen();
	start_series();
	start_hotspots();

	mouse_init_cycle();

	while (going && !g_engine->shouldQuit()) {
		if (keys_any()) {
			mykey = keys_get();
			switch (toupper(mykey)) {
			case esc_key:
				selected_item = 5;
				going = false;
				break;

			case alt_x_key:
			case alt_q_key:
			case ctrl_x_key:
			case ctrl_q_key:
				going = false;
				break;

			case 'S':
				menu_mode = MENU_APPEARING;
				menu_state = MENU_HIGH_SPRITE;
				animating_item = 0;
				menu_appear_state = 0;
				break;

			default:
				if (menu_mode == MENU_APPEARING)
					menu_mode = MENU_ACCEPTING_COMMANDS;
				break;
			}
		}

		mouse_begin_cycle(false);

		if (mouse_stroke_going) {
			if (menu_mode == MENU_APPEARING)
				menu_mode = MENU_ACCEPTING_COMMANDS;
		}

		now_clock = timer_read();
		kernel.clock = now_clock;

		if (now_clock >= menu_clock) {
			switch (menu_mode) {
			case MENU_APPEARING:
				if (--menu_state <= 1) {
					menu_state = MENU_HIGH_SPRITE;
					while (++animating_item < NUM_MENU_ITEMS && !menu_item[animating_item].active) {
					}

					if (animating_item >= NUM_MENU_ITEMS)
						menu_mode = MENU_ACCEPTING_COMMANDS;
				}
				break;

			case MENU_DISAPPEARING:
				if (++menu_state > (MENU_HIGH_SPRITE + 1))
					going = false;
				break;

			default:
				break;
			}

			menu_clock = now_clock + MENU_FRAME_RATE;
		}

		process_menu();

		if (now_clock >= frame_clock) {
			process_sprites();

			fx = new_background ? 1 : 0;
			matte_frame(fx, false);

			if (fx) {
				now_clock = timer_read();
				menu_clock = now_clock + MENU_FRAME_RATE;
			}

			must_perform_matte = false;
			new_background = false;

			frame_clock = now_clock + FRAME_RATE;
		}

		mouse_end_cycle(false, going);
	}

	stop_series();
}

} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
