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

#ifndef GLK_QUEST_GEAS_RUNNER
#define GLK_QUEST_GEAS_RUNNER

#include "glk/quest/string.h"
#include "glk/quest/geas_state.h"
#include "common/array.h"
#include "common/stream.h"

namespace Glk {
namespace Quest {

typedef Common::Array<String> vstring;
typedef Common::Array<vstring> v2string;

enum geas_justification { JUSTIFY_LEFT, JUSTIFY_RIGHT, JUSTIFY_CENTER };

struct GeasFontStyle {
	bool is_underlined, is_italic, is_bold;
	String color, font;
	int size;
	geas_justification justify;

	GeasFontStyle() : is_underlined(false), is_italic(false), is_bold(false),
		color(""), font(""), size(10), justify(JUSTIFY_LEFT) {}
};

class GeasFontStyleCompare {
public:
	int operator()(const GeasFontStyle &a, const GeasFontStyle &b) {
		if (a.size           !=  b.size)           return a.size < b.size;
		if (a.is_underlined  !=  b.is_underlined)  return a.is_underlined;
		if (a.is_bold        !=  b.is_bold)        return a.is_bold;
		if (a.is_italic      !=  b.is_italic)      return a.is_italic;
		if (a.color          !=  b.color)          return a.color < b.color;
		if (a.justify        !=  b.justify)        return a.justify < b.justify;
		return 0;
	}
};

Common::WriteStream &operator<< (Common::WriteStream &o, const GeasFontStyle &gfs);

enum GeasResult {
	r_success,
	r_failure,
	r_not_supported
};

/*  Callback object used to pass information from GeasCore
 *  to the interface objects
 */
class GeasInterface {
private:
	GeasFontStyle cur_style;
	String default_font;
	int default_size;
	//string fgcolor, bgcolor;

public:
	/* Takes 1 argument, a string with Quest markup
	 * Will output it to the user interface
	 * If the with_newline flag is set, it will print a newline afterwords
	 *     unless the string ends in "|xn"
	 */
	GeasResult print_formatted(String s, bool with_newline = true);

	/* Takes one argument; that string is printed without interpretation
	 * Must be implemented
	 * Called by print_formatted and by Geas directly.
	 */
	virtual GeasResult print_normal(const String &s) = 0;

	virtual GeasResult print_newline() = 0;

protected:

	void update_style() {
		set_style(cur_style);
	}

	/* Changes style of output text.
	 * Need not be implemented
	 * Only called by update_style()
	 */
	virtual GeasResult set_style(const GeasFontStyle &) {
		return r_not_supported;
	}

public:
	virtual String absolute_name(String rel_name, String parent) const = 0;
	virtual String get_file(const String &filename) const = 0;
	virtual void debug_print(const String &s) {
		warning("%s", s.c_str());
	}
	virtual GeasResult wait_keypress(String) {
		return r_not_supported;
	}
	virtual GeasResult pause(int msec) {
		return r_not_supported;
	}
	virtual GeasResult clear_screen() {
		return r_not_supported;
	}

	//virtual GeasResult set_foreground (string) { return r_not_supported; }
	//virtual GeasResult set_background (string) { return r_not_supported; }
	virtual void set_foreground(String) = 0;
	virtual void set_background(String) = 0;
	void set_default_font_size(String s);
	void set_default_font(String s);

	/* Unsure what arguments this will take.
	 * May also add animated, persistent, close image
	 */
	virtual GeasResult show_image(String filename, String resolution,
	                              String caption, ...) {
		return r_not_supported;
	}

	/* Again, unsure what arguments to give
	 * May add sound type
	 * If sync is true, do not return until file ends
	 * If filename is "", stop playing sounds.
	 */
	virtual GeasResult play_sound(String filename, bool looped, bool sync) {
		return r_not_supported;
	}

	/* Asks the user to type a free format string
	 */
	virtual String get_string() = 0;

	/* Presents a list with header 'info', and prompts the user to
	 * choose one item from 'choices'.
	 * returns the index chosen.
	 */
	virtual uint make_choice(String info, Common::Array<String> choices) = 0;

	/* Asks the user a yes/no question
	 * (If not overridden, this has an implementation that uses make_choice()
	 */
	virtual bool choose_yes_no(String question);

	/* args holds arguments sent to program.
	 * if active is true, geas should retain focus
	 * returns -   0 if disallowed
	 *         -   1 if succeeded
	 *         -   2 if file not found
	 *         -   3 if it couldn't find a program to run it
	 *         -   4 if it ran out of memory
	 */
	virtual int shell(Common::Array<String> args, bool active) {
		return 0;
	}

	/* say the argument using text-to-speech
	 */
	virtual GeasResult speak(String) {
		return r_not_supported;
	}

	virtual ~GeasInterface() {}

	/* This is a notification that some object has changed, and
	 * the interpreter may want to update the inventory or room object
	 * listings.
	 */
	virtual void update_sidebars() { }
};


/* Callback for passing information from the UI to the execution core
 */
class GeasRunner {
protected:
	GeasInterface *gi;

public:
	GeasRunner(GeasInterface *_gi) : gi(_gi) {}

	virtual bool is_running() const = 0;
	virtual GeasState *getState() = 0;
	virtual String get_banner() = 0;
	virtual void run_command(String) = 0;

	virtual v2string get_inventory() = 0;
	virtual v2string get_room_contents() = 0;
	virtual vstring  get_status_vars() = 0;
	virtual Common::Array<bool> get_valid_exits() = 0;

	virtual void tick_timers() = 0;

	virtual ~GeasRunner() {  }
	virtual void set_game(const String &fname) = 0;
	static GeasRunner *get_runner(GeasInterface *gi);
};

} // End of namespace Quest
} // End of namespace Glk

#endif
