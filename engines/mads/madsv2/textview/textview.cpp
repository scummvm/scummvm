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
#include "mads/madsv2/textview/textview.h"
#include "mads/madsv2/engine.h"

namespace MADS {
namespace MADSV2 {
namespace TextView {

constexpr int LINES_COUNT = 20;
struct TextViewLine {
	bool active;
	int16 x, y;
	char text[80];
};

Common::SeekableReadStream *file_handle;
static Room *room;
static TextViewLine lines[LINES_COUNT];
static bool isGoing, active, flag2, flag3, isEnd;
static bool flag5;
static long timer1, timer2, curr_time;
static char line_buffer[80];
static bool has_background, pan_flag;
static int peel_time, peel_x, peel_y;
static int text_x, text_y;

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

static void handle_command() {

}

static void position_line() {

}

static void shift_line(int lineNum) {

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

			if (flag2 && file_handle->eos() && !isEnd) {
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

			if (curr_time < timer2)
				matte_frame(0, 0);
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
			flag3 = has_background = false;
			timer2 = curr_time + 6;
		}
	} while (isGoing && !g_engine->shouldQuit());
}

void textview_main(const char *resName) {
	active = false;
	isGoing = flag2 = true;
	isEnd = flag3 = false;
	timer1 = timer2 = 0;
	has_background = pan_flag = false;
	flag5 = 0;
	text_x = text_y = 0;

	file_handle = env_open(resName);
	if (!file_handle)
		error("textview -- failed to open response file : %s", resName);

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
	auto &screen = *g_engine->getScreen();
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
