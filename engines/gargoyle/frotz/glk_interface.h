/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef GARGOYLE_FROTZ_GLK_INTERFACE
#define GARGOYLE_FROTZ_GLK_INTERFACE

#include "gargoyle/glk.h"
#include "gargoyle/frotz/mem.h"

namespace Gargoyle {
namespace Frotz {

enum SoundEffect {
	EFFECT_PREPARE     = 1,
	EFFECT_PLAY        = 2,
	EFFECT_STOP        = 3,
	EFFECT_FINISH_WITH = 4
};

enum RestartAction {
	RESTART_BEGIN = 0,
	RESTART_WPROP_SET = 1,
	RESTART_END = 2
};


/**
 * Implements an intermediate interface on top of the GLK layer, providing screen
 * and sound effect handling
 */
class GlkInterface : public Glk, public virtual UserOptions, public virtual Mem {
public:
	zchar statusline[256];
	int oldstyle;
	int curstyle;
	int cury;
	int curx;
	int fixforced;

	int curr_fg;
	int curr_bg;
	int curr_font;
	int prev_font;
	int temp_font;

	int curr_status_ht;
	int mach_status_ht;

	winid_t gos_status;
	winid_t gos_upper;
	winid_t gos_lower;
	winid_t gos_curwin;
	int gos_linepending;
	zchar *gos_linebuf;
	winid_t gos_linewin;
	schanid_t gos_channel;

	// Current window and mouse data
	int cwin;
	int mwin;
	int mouse_y;
	int mouse_x;
	int menu_selected;

	// IO streams
	bool ostream_screen;
	bool ostream_script;
	bool ostream_memory;
	bool ostream_record;
	bool istream_replay;
	bool message;

	// Window attributes
	bool enable_wrapping;
	bool enable_scripting;
	bool enable_scrolling;
	bool enable_buffering;

	// Sound fields
	int next_sample;
	int next_volume;

	bool _soundLocked;
	bool _soundPlaying;
protected:
	int os_char_width(zchar z);
	int os_string_width(const zchar *s);
	int os_string_length(zchar *s);
	void os_prepare_sample(int a);
	void os_finish_with_sample(int a);

	/**
	 * Play the given sample at the given volume (ranging from 1 to 8 and
	 * 255 meaning a default volume). The sound is played once or several
	 * times in the background (255 meaning forever). In Z-code 3 the
	 * repeats value is always 0 and the number of repeats is taken from
	 * the sound file itself. The end_of_sound function is called as soon
	 * as the sound finishes.
	 */
	void os_start_sample(int number, int volume, int repeats, zword eos);

	void os_stop_sample(int a);
	void os_beep(int volume);

	/**
	 * Call the IO interface to play a sample.
	 */
	void start_sample(int number, int volume, int repeats, zword eos);

	void start_next_sample();
	void gos_update_width();
	void gos_update_height();
	void reset_status_ht();
	void erase_window(zword w);
	void split_window(zword lines);
	void restart_screen();

	/**
	 * statusline overflowed the window size ... bad game!
	 * so ... split status text into regions, reformat and print anew.
	 */
	void packspaces(zchar *src, zchar *dst);

	void smartstatusline();

	/**
	 * Cancels any pending line
	 */
	void gos_cancel_pending_line();

	/**
	 * Called during game restarts
	 */
	void os_restart_game(RestartAction) {}

	/**
	 * Reads the mouse buttons
	 */
	zword os_read_mouse() {
		// Not implemented
		return 0;
	}
public:
	/**
	 * Constructor
	 */
	GlkInterface(OSystem *syst, const GargoyleGameDescription *gameDesc);

	/**
	 * Initialization
	 */
	void initialize();
};

} // End of namespace Frotz
} // End of namespace Gargoyle

#endif
