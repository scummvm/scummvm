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

#ifndef GLK_ZCODE_GLK_INTERFACE
#define GLK_ZCODE_GLK_INTERFACE

#include "glk/glk_api.h"
#include "glk/zcode/mem.h"
#include "glk/zcode/windows.h"

namespace Glk {
namespace ZCode {

#define zB(i) ((((i >> 10) & 0x1F) << 3) | (((i >> 10) & 0x1F) >> 2))
#define zG(i) ((((i >>  5) & 0x1F) << 3) | (((i >>  5) & 0x1F) >> 2))
#define zR(i) ((((i      ) & 0x1F) << 3) | (((i      ) & 0x1F) >> 2))
#define zRGB(i) _screen->format.RGBToColor(zR(i), zG(i), zB(i))
#define zcolor_NUMCOLORS    (13)

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

class Pics;

/**
 * Implements an intermediate interface on top of the GLK layer, providing screen
 * and sound effect handling
 */
class GlkInterface : public GlkAPI, public virtual UserOptions, public virtual Mem {
private:
	bool _reverseVideo;
public:
	Pics *_pics;
	zchar statusline[256];
	uint zcolors[zcolor_NUMCOLORS];
	int fixforced;

	int curr_status_ht;
	int mach_status_ht;

	Windows _wp;
	winid_t gos_status;
	int gos_linepending;
	zchar *gos_linebuf;
	winid_t gos_linewin;
	schanid_t gos_channel;

	// Mouse data
	int mwin;
	int mouse_y;
	int mouse_x;
	int menu_selected;

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
private:
	/**
	 * Loads the pictures file for Infocom V6 games
	 */
	bool initPictures();

	/**
	 * Displays the title screen for the game Beyond Zork
	 */
	void showBeyondZorkTitle();

	/**
	 * Add any Sound subfolder or sound zip file for access
	 */
	void addSound();

	/**
	 * Do a rounding division, rounding to even if fraction part is 1/2.
	 */
	uint roundDiv(uint x, uint y);
protected:
	/**
	 * Return the length of the character in screen units.
	 */
	int os_char_width(zchar z);

	/**
	 * Calculate the length of a word in screen units. Apart from letters,
	 * the word may contain special codes:
	 *
	 *    ZC_NEW_STYLE - next character is a new text style
	 *    ZC_NEW_FONT  - next character is a new font
	 */
	int os_string_width(const zchar *s);

	/**
	 * Return the length of a string
	 */
	int os_string_length(zchar *s);

	/**
	 * Prepare a sample for playing
	 */
	void os_prepare_sample(int a);

	/**
	 * Signal that a given sample is finished with
	 */
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

	/**
	 * Stop playing a given sound number
	 */
	void os_stop_sample(int a);

	/**
	 * Make a beep sound
	 */
	void os_beep(int volume);

	/**
	 * Return true if the given picture is available. If so, write the
	 * width and height of the picture into the appropriate variables.
	 * Only when picture 0 is asked for, write the number of available
	 * pictures and the release number instead.
	 */
	bool os_picture_data(int picture, uint *height, uint *width);

	/**
	 * Display a picture at the given coordinates. Top left is (1,1).
	 */
	void os_draw_picture(int picture, const Common::Point &pos);

	/**
	 * Return the colour of the pixel below the cursor. This is used by V6 games to print
	 * text on top of pictures. The coulor need not be in the standard set of Z-machine colours.
	 */
	int os_peek_color();

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
	void os_restart_game(RestartAction stage) {}

	/**
	 * Reads the mouse buttons
	 */
	zword os_read_mouse() {
		// Not implemented
		return 0;
	}

	void os_scrollback_char(zchar z) {
		// Not implemented
	}

	void os_scrollback_erase(int amount) {
		// Not implemented
	}

	/**
	 * Waits for a keypress
	 */
	zchar os_read_key(int timeout, bool show_cursor);

	/**
	 * Waits for the user to type an input line
	 */
	zchar os_read_line(int max, zchar *buf, int timeout, int width, int continued);
public:
	/**
	 * Constructor
	 */
	GlkInterface(OSystem *syst, const GlkGameDescription &gameDesc);

	/**
	 * Destructor
	 */
	~GlkInterface() override;

	/**
	 * Initialization
	 */
	void initialize();
};

} // End of namespace ZCode
} // End of namespace Glk

#endif
