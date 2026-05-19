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

#include "common/textconsole.h"
#include "mads/madsv2/core/cycle.h"
#include "mads/madsv2/core/env.h"
#include "mads/madsv2/core/font.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/mcga.h"
#include "mads/madsv2/core/mem.h"
#include "mads/madsv2/core/mouse.h"
#include "mads/madsv2/core/pal.h"
#include "mads/madsv2/core/room.h"
#include "mads/madsv2/core/timer.h"
#include "mads/madsv2/core/video.h"
#include "mads/madsv2/textview/textview.h"
#include "mads/madsv2/engine.h"

namespace MADS {
namespace MADSV2 {
namespace TextView {

constexpr int LINES_COUNT = 20;
struct TextViewLine {
	int16 active;
	int16 x, y;
	char text[80];
};

Common::SeekableReadStream *file_handle;
static Room *room;
static TextViewLine lines[LINES_COUNT];
static bool isGoing, active, flag2, flag3;
static bool flag4, flag5, isEnd;
static int16 font_auto_spacing;
static long timer1, timer2, timer3, curr_time;
static char line_buffer[80];
static char command_buffer[80];
static bool has_background, pan_flag;
static int16 peel_time, peel_x, peel_y;
static int16 text_x, text_y;
static int16 room_id;
static bool spare[3];
static Buffer room_picture[3];
static Buffer *background_ptr;
static int16 xPos;
static byte line_slice[156];

static void read_line_slice(int xp) {
	const byte *src = buffer_pointer(background_ptr, xp, 0);
	byte *dest = line_slice;
	for (int y = 0; y < 156; ++y, ++dest, src += background_ptr->x)
		*dest = *src;
}

static void write_line_slice(int xp) {
	byte *src = buffer_pointer(&scr_orig, xp, 0);
	byte *dest = buffer_pointer(&scr_work, xp, 0);
	const byte *ref = line_slice;
	byte srcV, destV;

	for (int y = 0; y < 156; ++y, src += 320, dest += 320, ++ref) {
		srcV = *src;
		destV = *dest;
		*src = *ref;

		if (srcV == destV)
			*dest = *ref;
	}
}

static void textview_timer() {
	long timer = timer_read();

	if (flag4 && timer >= timer3) {
		if (xPos >= 320) {
			flag4 = false;
		} else {
			read_line_slice(xPos);
			write_line_slice(xPos);
			video_update(&scr_work, xPos, 0, xPos, viewing_at_y, 1, 156);
			++xPos;
			timer3 = timer + 1;
		}
	}
}

static void strip_linefeed(char *line) {
	char *lf = strrchr(line, '\n');
	if (lf)
		*lf = '\0';
}

static void trim_right(char *line) {
	Common::String s(line);
	while (!s.empty() && (s.lastChar() == ' ' || s.lastChar() == '\t'))
		s.deleteLastChar();

	Common::strcpy_s(line, 80, s.c_str());
}

static bool check_command(const char *cmd, const char **value = nullptr) {
	size_t len = strlen(cmd);
	if (value)
		*value = command_buffer + len;

	return strncmp(command_buffer, cmd, len) == 0;
}

static void find_number(const char *&value) {
	while (*value && !Common::isDigit(*value))
		++value;
}

static void find_next_number(const char *&value) {
	if (*value) {
		while (*value && *value != ',')
			++value;
	}
	if (*value) {
		while (*value && !Common::isDigit(*value))
			++value;
	}
}

static void load_background(const char *value) {
	find_number(value);
	room_id = atoi(value);
	has_background = true;
	flag5 = false;
	active = false;

	pal_init(8, 8);
	pal_white(master_palette);
	master_palette[5].r = 0;
	master_palette[5].g = 63;
	master_palette[5].b = 63;
	master_palette[6].r = 0;
	master_palette[6].g = 45;
	master_palette[6].b = 45;

	room = room_load(room_id, 0, nullptr, &scr_orig, &scr_depth, &scr_walk,
		&scr_special, &picture_map, &depth_map, &picture_resource,
		&depth_resource, -1, -1, 0);
	if (!room)
		error("Could not load room %d", room_id);

	image_marker = 1;
	image_list[0].flags = IMAGE_REFRESH;
	image_list[0].segment_id = 0xff;
}

static void pan(const char *value) {
	find_number(value);
	peel_x = atoi(value);
	find_next_number(value);
	peel_y = atoi(value);
	find_next_number(value);
	peel_time = atoi(value);

	pan_flag = peel_x != 0 || peel_y != 0;
}

static void sound_driver(const char *value) {
	g_engine->_soundManager->removeDriver();

	char num = value[strlen(value) - 1];
	if (Common::isDigit(num))
		g_engine->_soundManager->init(num - '0');
}

static void play_sound(const char *value) {
	find_number(value);
	int num = atoi(value);

	if (g_engine->_soundManager->isLoaded())
		g_engine->_soundManager->command(num);
}

static void set_color(int index, const char *value) {
	find_number(value);
	master_palette[5 + index].r = atoi(value);
	find_next_number(value);
	master_palette[5 + index].g = atoi(value);
	find_next_number(value);
	master_palette[5 + index].b = atoi(value);

	mcga_setpal_range(&master_palette, 5 + index, 1);
}

static void set_spare(const char *value) {
	find_number(value);
	int index = atoi(value);
	find_next_number(value);
	int roomId = atoi(value);

	if (spare[index]) {
		buffer_free(&room_picture[index]);
		spare[index] = false;
	}

	spare[index] = room_picture_load(roomId, &room_picture[index], 0) == 0;
}

static void set_page(const char *value) {
	find_number(value);
	int index = atoi(value);

	if (spare[index] && !flag4) {
		background_ptr = &room_picture[index];
		flag4 = true;
		xPos = 0;
		timer3 = timer_read();
		timer_activate_low_priority(textview_timer);
	}
}

static void handle_command() {
	const char *value = nullptr;

	// Get end of command
	*command_buffer = '\0';
	char *end = strchr(line_buffer, ']');
	if (!end)
		end = line_buffer + strlen(line_buffer);
	char endChar = *end;
	*end = '\0';

	// Copy out the command
	Common::strcpy_s(command_buffer, line_buffer + 1);
	mads_strupr(command_buffer);

	*end = endChar;
	if (endChar)
		Common::strcpy_s(line_buffer, end + 1);

	if (check_command("BACKGROUND", &value))
		load_background(value);
	if (check_command("GO"))
		active = true;
	if (check_command("PAN", &value))
		pan(value);
	if (check_command("DRIVER", &value))
		sound_driver(value);
	if (check_command("SOUND", &value))
		play_sound(value);
	if (check_command("COLOR0", &value))
		set_color(0, value);
	if (check_command("COLOR1", &value))
		set_color(1, value);
	if (check_command("SPARE", &value))
		set_spare(value);
	if (check_command("PAGE", &value))
		set_page(value);
}

static void position_line() {
	char tmp_buf[84];

	if ((int)font_conv->max_y_size + text_y + 2 >= 156)
		return;

	// Split line_buffer at first '[' (start of commands) or at the null terminator
	char *split = strchr(line_buffer, '[');
	if (!split)
		split = line_buffer + strlen(line_buffer);

	// Copy text portion (before split) to tmp_buf, then shift remainder to line_buffer start
	char saved = *split;
	*split = '\0';
	Common::strcpy_s(tmp_buf, line_buffer);
	*split = saved;
	Common::strcpy_s(line_buffer, split);

	if (!strlen(line_buffer))
		flag2 = flag3 = true;

	// Handle '@' horizontal-position marker
	char *at = strchr(tmp_buf, '@');
	int width;
	if (at) {
		*at = '\0';
		int left_width = font_string_width(font_conv, tmp_buf, font_auto_spacing);
		memmove(at, at + 1, strlen(at + 1) + 1);
		if (text_x == -1)
			text_x = 160 - left_width;
		width = font_string_width(font_conv, tmp_buf, font_auto_spacing);
	} else {
		width = font_string_width(font_conv, tmp_buf, font_auto_spacing);
		if (text_x == -1)
			text_x = 160 - width / 2;
	}

	// Find first inactive slot in lines[]
	int slot = -1;
	for (int i = 0; i < LINES_COUNT; ++i) {
		if (!lines[i].active) {
			slot = i;
			break;
		}
	}

	if (slot >= 0) {
		if (text_x < 0 || text_x + width - 1 >= 320)
			warning("position_line: text out of bounds: text_x=%d width=%d", text_x, width);

		lines[slot].active = -1;
		lines[slot].x = text_x;
		lines[slot].y = 155;
		Common::strcpy_s(lines[slot].text, tmp_buf);
	}

	text_x = -1;
}

static void shift_line(int lineNum) {
	int fontH = font_conv->max_y_size;
	TextViewLine &line = lines[lineNum];

	if ((line.y + fontH) < 0) {
		line.active = false;
	} else {
		matte_add_message(font_conv, line.text, line.x, line.y, 0x605, font_auto_spacing);
		line.y--;
	}
}

static void process_line() {
	while (*line_buffer == '[')
		handle_command();

	if (flag5)
		position_line();
	else
		flag2 = true;

	if (active) {
		active = false;
		flag5 = true;
	}
}

static void peel() {
	if (peel_x) {
		buffer_peel_horiz(&scr_orig, peel_x);
		matte_refresh_work();
	}

	if (peel_y) {
		buffer_peel_vert(&scr_orig, peel_y, scr_work.data, scr_work.x * scr_work.y);
		matte_refresh_work();
	}
}

static void animate() {
	do {
		if (g_engine->hasPendingKey()) {
			g_engine->flushKeys();
			isGoing = false;
			break;
		}

		do {
			if (flag3)
				continue;

			if (flag2 && !file_handle->eos() && !isEnd) {
				Common::String s = file_handle->readLine();
				Common::strcpy_s(line_buffer, s.c_str());
				strip_linefeed(line_buffer);
				trim_right(line_buffer);
				flag2 = false;

				isEnd = file_handle->eos();
				if (!strncmp(line_buffer, "***", 3)) {
					// End of text
					isEnd = flag2 = true;
				}
			}

			if (!flag2)
				process_line();

		} while (flag2 && !flag3 && !isEnd);

		curr_time = timer_read();

		if (pan_flag && !has_background && curr_time >= timer1 && curr_time <= timer2) {
			peel();
			timer1 = curr_time + peel_time;
			curr_time = timer_read();

			if (curr_time < timer2) {
				matte_frame(0, 0);
				mouse_hide();
			}
		}

		if (curr_time >= timer2) {
			int total = 0;
			text_y = 0;

			for (auto &msg : message_list) {
				if (msg.active)
					msg.status = -1;
			}
			for (int index = 0; index < LINES_COUNT; ++index) {
				auto &line = lines[index];
				if (line.active) {
					++total;
					shift_line(index);
					text_y = MAX<int>(text_y, line.y);
				}
			}

			if (isEnd && total == 0)
				isGoing = false;

			matte_frame(has_background ? 2 : 0, 0);
			mouse_hide();

			flag3 = has_background = false;
			timer2 = curr_time + 6;
		}

		g_system->delayMillis(10);
	} while (isGoing && !g_engine->shouldQuit());
}

void textview_main(const char *resName) {
	active = false;
	isGoing = flag2 = true;
	isEnd = flag3 = false;
	flag4 = false;
	timer1 = timer2 = timer3 = 0;
	has_background = pan_flag = false;
	flag5 = 0;
	text_x = text_y = 0;
	font_auto_spacing = -1;
	room_id = 0;
	memset(spare, 0, sizeof(spare));
	memset(&room_picture, 0, sizeof(room_picture));
	background_ptr = nullptr;
	xPos = 0;

	// Clear the screen
	auto &screen = *g_engine->getScreen();
	screen.clear();
	mouse_hide();

	// Open the text resource file
	Common::String fname = Common::String::format("*%s.txr", resName);
	file_handle = env_open(fname.c_str());
	if (!file_handle)
		error("textview -- failed to open response file : %s", fname.c_str());

	font_conv = font_load("*fontconv.ff");
	if (!font_conv)
		error("textview -- failed to open font.");

	mcga_compute_retrace_parameters();
	memset(cycling_palette, 0, sizeof(cycling_palette));
	cycle_list.num_cycles = 0;

	// Initialize surfaces
	pal_init(8, 8);
	pal_white(master_palette);
	buffer_init(&scr_work, 320, 156);
	buffer_init(&scr_depth, 320, 156);
	buffer_init(&scr_orig, 320, 156);
	assert(scr_work.data && scr_depth.data && scr_orig.data);

	viewing_at_y = (200 - scr_work.y) / 2;
	room = nullptr;

	// Clear buffers and palette
	buffer_fill(scr_work, 0);
	buffer_fill(scr_orig, 0);
	buffer_fill(scr_depth, 0xff);
	mcga_setpal(&master_palette);

	mouse_set_work_buffer(scr_work.data, scr_work.x);
	mouse_set_view_port_loc(0, viewing_at_y, scr_work.x, scr_work.y + viewing_at_y - 1);
	mouse_set_view_port(0, 0);
	timer_install();
	matte_init(-1);
	timer_activate_low_priority(cycle_colors);

	// Draw boundary horizontal lines at top and bottom of the screen
	if (viewing_at_y) {
		screen.hLine(0, viewing_at_y - 2, 319, 2);
		screen.hLine(0, viewing_at_y + scr_work.y + 1, 319, 2);
	}

	// Clear the lines
	memset(lines, 0, sizeof(lines));

	// Handle the loading and animation of the text lines
	animate();

	// Teardown
	g_engine->_soundManager->removeDriver();
	pal_interface(master_palette);
	mcga_setpal(&master_palette);
	mcga_reset();

	buffer_free(&scr_depth);
	buffer_free(&scr_orig);
	buffer_free(&scr_work);
	mem_free(font_conv);

	delete file_handle;
}

} // namespace TextView
} // namespace MADSV2
} // namespace MADS
