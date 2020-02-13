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

#ifndef GLK_QUEST_GEAS_GLK
#define GLK_QUEST_GEAS_GLK

#include "glk/quest/geas_runner.h"
#include "glk/windows.h"

namespace Glk {
namespace Quest {


/* User interface bridge from Geas Core to Glk.

  Glk Window arrangment.

    +---------+
    |    B    |
    +---------+
    |    M    |
    |         |
    +---------+
    |    I    |
    +---------+

  B is a one line "banner window", showing the game name and author.  Kept
  in the global variable, it's optional, null if unavailable.
  optional.
  M is the main window where the text of the game appears.  Kept in the
  global variable mainglkwin.
  I is a one line "input window" where the user inputs their commands.
  Kept in the global variable inputwin, it's optional, and if not separate
  is set to mainglkwin.

  Maybe in future revisions there will be a status window (including a
  compass rose).
*/

class GeasGlkInterface : public GeasInterface {
protected:
	String get_file(const String &fname) const override;
	GeasResult print_normal(const String &s) override;
	GeasResult print_newline() override;

	void set_foreground(String) override;
	void set_background(String) override;
	GeasResult set_style(const GeasFontStyle &) override;

	String get_string() override;
	uint make_choice(String, Common::Array<String>) override;

	String absolute_name(String, String) const override;
public:
	GeasGlkInterface() {
		;
	}
};

extern winid_t mainglkwin;
extern winid_t inputwin;
extern winid_t bannerwin;
extern strid_t inputwinstream;
extern int ignore_lines;
extern const bool use_inputwindow;

extern void glk_put_cstring(const char *);
extern void draw_banner();

} // End of namespace Quest
} // End of namespace Glk

#endif
