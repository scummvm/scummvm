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

#include "mads/madsv2/engine.h"
#include "mads/madsv2/core/general.h"
#include "mads/madsv2/core/config.h"
#include "mads/madsv2/core/font.h"
#include "mads/madsv2/core/speech.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/sprite.h"
#include "mads/madsv2/core/error.h"
#include "mads/madsv2/core/room.h"
#include "mads/madsv2/core/fileio.h"
#include "mads/madsv2/core/env.h"
#include "mads/madsv2/core/buffer.h"
#include "mads/madsv2/core/mem.h"
#include "mads/madsv2/core/pal.h"
#include "mads/madsv2/core/timer.h"
#include "mads/madsv2/core/mcga.h"
#include "mads/madsv2/core/cycle.h"
#include "mads/madsv2/core/mouse.h"
#include "mads/madsv2/core/screen.h"
#include "mads/madsv2/core/video.h"
#include "mads/madsv2/core/lib.h"
#include "mads/madsv2/core/keys.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/btype.h"
#include "mads/madsv2/core/hspot.h"
#include "mads/madsv2/core/magic.h"
#include "mads/madsv2/core/ems.h"
#include "mads/madsv2/core/xms.h"
#include "mads/madsv2/core/imath.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/pack.h"
#include "mads/madsv2/core/quote.h"
#include "mads/madsv2/phantom/mads/sounds.h"
#include "mads/madsv2/phantom/mads/quotes.h"
#include "mads/madsv2/phantom/mads/speeches.h"
#include "mads/madsv2/phantom/main_menu.h"

namespace MADS {
namespace MADSV2 {
namespace Phantom {

#define menu_version    "1.08"
#define menu_date       "26-May-93"

#define WIN             16
#define WIN2            32
#define WIN3            48

#define FIRST_TIME_FILE  "PHANTOM.RTN"
#define CONFIG_FILE_NAME "CONFIG.PHA"

#define EYE_QUOTES      16
#define EYE_HOTSPOT     32
#define EYE_MESSAGES    2

#undef MESSAGE_COLOR
#define MESSAGE_COLOR   1284


ConfigFile config_file;

int mads_mode = false;

FontPtr font = NULL;
int font_auto_spacing = -1;

bool new_background   = false;
int room_number       = 0;

int win_status        = 0;

bool force_opening    = false;
bool dont_return      = false;
bool white_bars       = true;
byte white_border     = 2;

char command_line[COMMAND_LINE_MAX][80];
char *command_list[COMMAND_LINE_MAX+1];
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
int next_x    = -1;

char sound_file[80]    = "*#SOUND.PH9";
char sound_letter      = 'N';

int  sound_board_address = 0x220;              /* Default sound board address */
int  sound_board_type    = sound_board_roland; /* Default sound board type    */

int  swapping = false;                   /* Flag when swapping in new background */
int  swap_line;                          /* Currently active swap line           */
long swap_clock;                         /* Time for next swap */

int  current_item;
int  current_eye   = false;
int  eye_latch     = false;
int  selected_item = -1;

#define LEFT_EYE   0
#define RIGHT_EYE  1

extern char *quotes;
int  eye_message[2];
int  eye_pokes   = 0;
int  recent_eye  = 0;
int  rebel_base  = 0;
int  poke_count_message = -1;
int  poke_count         = 0;
int  poke_counting      = false;

char poke_count_buf[2][20];

char bonus_buf[80] = "";

Palette special_pal;                     /* Palette for fadeout */


MenuItem menu_item[NUM_MENU_ITEMS+1];    /* Menu item array */

void start_series() {
	int error_flag = true;
	int count;
	int handle;
	char temp_buf[80];

	for (count = 0; count < NUM_MENU_ITEMS; count++) {
		handle = -1;
		Common::strcpy_s(temp_buf, "*MAIN0.SS");
		temp_buf[5] = (char)('0' + count);

		handle = matte_load_series(temp_buf, 0, 0);
		if (handle < 0) goto done;

		menu_item[count].handle = handle;
		menu_item[count].active = true;
		menu_item[count].status = 0;
	}

	if (menu_item[6].active) {
		/* menu_item[6].active = config_file.quotes_enabled; */
		menu_item[6].active = false;
	}

	error_flag = false;

done:
	if (error_flag) {
		Common::strcpy_s(error_string, temp_buf);
		error_report(ERROR_SERIES_LOAD_FAILED, WARNING, MODULE_UNKNOWN, count, handle);
	}
}

void stop_series(void) {
	int count;

	for (count = NUM_MENU_ITEMS - 1; count >= 0; count--) {
		matte_deallocate_series(menu_item[count].handle, true);
	}
}

void start_hotspots(void) {
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

	hspot_add(135, 77, 148, 83, 2, EYE_HOTSPOT, mcga_mode);
	hspot_add(156, 77, 170, 83, 2, EYE_HOTSPOT + 1, mcga_mode);
}

void process_menu(void) {
	int myspot;

	myspot = hspot_which(mouse_x, mouse_y - viewing_at_y, mcga_mode);

	current_eye = false;

	if ((myspot > 0) && mouse_any_stroke && (menu_mode == MENU_ACCEPTING_COMMANDS)) {
		current_item = spot[myspot].num;
		if (current_item >= EYE_HOTSPOT) {
			current_item = -1;
			current_eye = true;
		}
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

void process_sprites(void) {
	int count;
	int sprite;
	int series;
	int look, match;
	Image image;

	for (count = 0; count < (int)image_marker; count++) {
		if ((image_list[count].segment_id < KERNEL_SEGMENT_ANIMATION) ||
			(image_list[count].segment_id > KERNEL_SEGMENT_ANIMATION_HIGH)) {
			if (image_list[count].flags >= IMAGE_STATIC) {
				image_list[count].flags = IMAGE_ERASE;
			}
		}
	}

	if (menu_mode == MENU_APPEARING) goto done;

	for (count = 0; count < NUM_MENU_ITEMS; count++) {
		if (menu_item[count].active) {
			image.flags = IMAGE_UPDATE;
			image.segment_id = (byte)(count + 1);

			series = count;
			if (menu_mode == MENU_ACCEPTING_COMMANDS) {
				if (count != current_item) {
					sprite = 1;
				} else {
					sprite = 2;
				}
			} else {
				sprite = 1;
				/*
				if (count != selected_item) {
				  sprite = 1;
				} else {
				  sprite = 2;
				}
				*/
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
					if (memcmp(&image_list[look].series_id,
						&image.series_id, 9) == 0) {
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

done:
	;
}

void process_messages(int anim) {
	int total;
	int count;
	int id, x, y;
	int flags;
	int special_check;
	int buf;

	if (!current_eye) goto done;
	if (eye_latch)    goto done;

	total = 0;

	for (count = 0; count < KERNEL_MAX_MESSAGES; count++) {
		if (kernel_message[count].flags) total++;
	}

	if (total > 6) goto done;

	eye_pokes++;

	if (eye_pokes < 6) {
		id = eye_pokes - 1;
	} else {
		id = imath_random(0, EYE_QUOTES - 1);
	}

	if (poke_counting) {
		if (poke_count_message >= 0) {
			kernel_message_delete(poke_count_message);
		}
		poke_count++;
		buf = (poke_count & 1);
		Common::sprintf_s(poke_count_buf[buf], "%d", poke_count);
		poke_count_message = kernel_message_add(poke_count_buf[buf], 160, 40,
			MESSAGE_COLOR, 180, 0, KERNEL_MESSAGE_CENTER);
	}

	id += quote_mainmenu_phantom_1;

	recent_eye = 1 - recent_eye;

	if (((total > 1) && !poke_counting) || (total > 2)) {
		if (!poke_counting || (eye_message[recent_eye] != poke_count_message)) {
			kernel_message_delete(eye_message[recent_eye]);
		}
	}

	if (recent_eye == LEFT_EYE) {
		x = 120;
		flags = KERNEL_MESSAGE_RIGHT;
	} else {
		x = 184;
		flags = 0;
	}

	special_check = eye_pokes % 4;
	if ((special_check == 1) || (special_check == 2)) {
		y = 90;
	} else {
		y = 66;
	}

	eye_message[recent_eye] = kernel_message_add(quote_string(quotes, id), x, y,
		MESSAGE_COLOR, 180, 0, flags);

done:
	eye_latch = current_eye;

	if (current_eye) {
		kernel_reset_animation(anim, 140);
		kernel_anim[anim].next_clock = kernel.clock;
	}
}

void menu_control(void) {
	int fx;
	int mykey;
	int last_frame = -1;
	int now_frame;
	int reset_frame;
	int random;
	int anim;
	int initial_reset = false;

	menu_mode = MENU_APPEARING;
	menu_state = MENU_HIGH_SPRITE;
	menu_appear_state = 0;
	menu_clock = 999999999;

	going = true;
	must_perform_matte = false;

	frame_clock = 0;

	start_series();
	start_hotspots();

	anim = kernel_run_animation("*RM922A.AA", 0);

	mouse_init_cycle();

	while (going && !g_engine->shouldQuit()) {
		if (keys_any()) {
			mykey = keys_get();
			switch (toupper(mykey)) {
			case esc_key:
				selected_item = 4;
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
				menu_appear_state = 0;
				break;

			default:
				if (menu_mode == MENU_APPEARING) {
					menu_mode = MENU_ACCEPTING_COMMANDS;
					kernel_reset_animation(anim, 136);
					kernel_anim[anim].next_clock = kernel.clock;
					global_speech_go(speech_phantom_cackle);
				}
				break;
			}
		}

		mouse_begin_cycle(false);

		if (mouse_stroke_going) {
			if (menu_mode == MENU_APPEARING) {
				menu_mode = MENU_ACCEPTING_COMMANDS;
				global_speech_go(speech_phantom_cackle);
				kernel_reset_animation(anim, 136);
				kernel_anim[anim].next_clock = kernel.clock;
				mouse_init_cycle();
			}
		}

		if (menu_mode != MENU_APPEARING) {
			mouse_show();
		}

		now_clock = timer_read();
		kernel.clock = now_clock;

		if (now_clock >= menu_clock) {
			switch (menu_mode) {
			case MENU_APPEARING:
				/*
				menu_state -= 1;
				if (menu_state <= 1) {
				  menu_state         = MENU_HIGH_SPRITE;
				  do {
					menu_appear_state += 1;
				  } while ((menu_appear_state < NUM_MENU_ITEMS) &&
					   (!menu_item[menu_appear_state].active));
				  if (menu_appear_state >= NUM_MENU_ITEMS) {
					menu_mode          = MENU_ACCEPTING_COMMANDS;
				  }
				}
				*/
				break;

			case MENU_ACCEPTING_COMMANDS:
				break;

			case MENU_DISAPPEARING:
				going = false;
				/*
				menu_state += 1;
				if (menu_state > MENU_HIGH_SPRITE + 1) {
				  going = false;
				}
				*/
				break;
			}
			menu_clock = now_clock + MENU_FRAME_RATE;
		}

		process_menu();
		kernel_message_update_all();
		kernel_process_all_animations();

		process_messages(anim);

		reset_frame = -1;
		now_frame = kernel_anim[anim].frame;
		if (now_frame != last_frame) {
			last_frame = now_frame;

			if (now_frame == 64) {
				global_speech_go(speech_phantom_cackle);
			}

			if (now_frame < 136) {
				if (menu_mode != MENU_APPEARING) {
					if (!initial_reset) {
						reset_frame = 136;
						initial_reset = true;
					}
				}
			}

			if (now_frame >= 161) {
				if (menu_mode == MENU_APPEARING) {
					menu_mode = MENU_ACCEPTING_COMMANDS;
					global_speech_go(speech_phantom_cackle);
				}

				random = imath_random(1, 1000);

				if (random <= 250) {
					reset_frame = 136;
				} else if (random <= 500) {
					reset_frame = 150;
				} else {
					reset_frame = 159;
				}

			}

			if (reset_frame >= 0) {
				kernel_reset_animation(anim, reset_frame);
				last_frame = reset_frame;
			}
		}

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

	kernel_abort_animation(anim);

	stop_series();
}

void add_parameter(char *parameter) {
	if (command_line_size < COMMAND_LINE_MAX) {
		Common::strcpy_s(command_line[command_line_size], parameter);
		command_list[command_line_size] = command_line[command_line_size];
		command_line_size++;
	}
}

void add_sound_parameter(void) {
	char temp_buf[80];
	char work_buf[80];

	Common::strcpy_s(temp_buf, "-c:n");
	temp_buf[3] = sound_letter;

	Common::strcat_s(temp_buf, ",");
	Common::sprintf_s(work_buf, "%x", config_file.sound_card_address);
	Common::strcat_s(temp_buf, work_buf);
	Common::strcat_s(temp_buf, ",");
	Common::sprintf_s(work_buf, "%x", config_file.sound_card_type);
	Common::strcat_s(temp_buf, work_buf);

	add_parameter(temp_buf);
}

void add_speech_parameter(void) {
	char temp_buf[80];
	char work_buf[80];

	if (config_file.speech_card_type > 0) {
		Common::strcpy_s(temp_buf, "-u:");

		Common::sprintf_s(work_buf, "%x", config_file.speech_card_address);
		Common::strcat_s(temp_buf, work_buf);
		Common::strcat_s(temp_buf, ",");
		Common::sprintf_s(work_buf, "%x", config_file.speech_card_type);
		Common::strcat_s(temp_buf, work_buf);
		Common::strcat_s(temp_buf, ",");
		env_catint(temp_buf, config_file.speech_card_irq, 2);
		Common::strcat_s(temp_buf, ",");
		env_catint(temp_buf, config_file.speech_card_drq, 2);

		add_parameter(temp_buf);
	}
}

void add_chain_parameter(void) {
	char temp_buf[80];
	int added_dash = false;

	Common::strcpy_s(temp_buf, "-a:\"mainmenu");

	if (env_search_mode == ENV_SEARCH_CONCAT_FILES) {
		Common::strcat_s(temp_buf, " -p");
		added_dash = true;
	}

	if (use_mouse_cursor_fix) {
		if (!added_dash) {
			Common::strcat_s(temp_buf, " -");
			added_dash = true;
		}
		Common::strcat_s(temp_buf, "u");
	}
}

} // namespace Phantom
} // namespace MADSV2
} // namespace MADS
