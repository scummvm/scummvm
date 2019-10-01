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

#include "glk/quest/geas_glk.h"
#include "glk/quest/geas_runner.h"
#include "glk/quest/quest.h"
#include "glk/quest/streams.h"
#include "glk/windows.h"

namespace Glk {
namespace Quest {

void glk_put_cstring(const char *);


winid_t mainglkwin;
winid_t inputwin;
winid_t bannerwin;
strid_t inputwinstream;

const bool use_inputwindow = false;

int ignore_lines;			// count of lines to ignore in game output

void draw_banner() {
	uint width;
	uint index;
	if (bannerwin) {
		g_vm->glk_window_clear(bannerwin);
		g_vm->glk_window_move_cursor(bannerwin, 0, 0);
		strid_t stream = g_vm->glk_window_get_stream(bannerwin);

		g_vm->glk_set_style_stream(stream, style_User1);
		g_vm->glk_window_get_size(bannerwin, &width, NULL);
		for (index = 0; index < width; index++)
			g_vm->glk_put_char_stream(stream, ' ');
		g_vm->glk_window_move_cursor(bannerwin, 1, 0);

		if (g_vm->banner.empty())
			g_vm->glk_put_string_stream(stream, "Geas 0.4");
		else
			g_vm->glk_put_string_stream(stream, g_vm->banner.c_str());
	}
}

void glk_put_cstring(const char *s) {
	if (!g_vm->loadingSavegame())
		g_vm->glk_put_string(s);
}

GeasResult GeasGlkInterface::print_normal(const String &s) {
	if (!ignore_lines)
		glk_put_cstring(s.c_str());
	return r_success;
}

GeasResult GeasGlkInterface::print_newline() {
	if (!ignore_lines)
		glk_put_cstring("\n");
	else
		ignore_lines--;
	return r_success;
}

GeasResult GeasGlkInterface::set_style(const GeasFontStyle &style) {
	// Glk styles are defined before the window opens, so at this point we can only
	// pick the most suitable style, not define a new one.
	uint match;
	if (style.is_italic && style.is_bold)
		match = style_Alert;
	else if (style.is_italic)
		match = style_Emphasized;
	else if (style.is_bold)
		match = style_Subheader;
	else if (style.is_underlined)
		match = style_User2;
	else
		match = style_Normal;

	g_vm->glk_set_style_stream(g_vm->glk_window_get_stream(mainglkwin), match);
	return r_success;
}

void GeasGlkInterface::set_foreground(String s) {
	if (s != "") {
	}
}

void GeasGlkInterface::set_background(String s) {
	if (s != "") {
	}
}

/* Code lifted from GeasWindow.  Should be common.  Maybe in
 * GeasInterface?
 */
String GeasGlkInterface::get_file(const String &fname) const {
	Common::File f;
	if (!f.open(fname)) {
		glk_put_cstring("Couldn't open ");
		glk_put_cstring(fname.c_str());
		g_vm->glk_put_char(0x0a);
		return "";
	}

	// Read entirety of the file
	char *buf = new char[f.size()];
	f.read(buf, f.size());
	
	String result(buf, buf + f.size());
	delete[] buf;

	return result;
}

String GeasGlkInterface::get_string() {
	char buf[200];
	g_vm->glk_request_line_event(inputwin, buf, (sizeof buf) - 1, 0);
	while (1) {
		event_t ev;

		g_vm->glk_select(&ev);

		if (ev.type == evtype_LineInput && ev.window == inputwin) {
			return String(buf, ev.val1);
		}
		/* All other events, including timer, are deliberately
		 * ignored.
		 */
	}
}

uint GeasGlkInterface::make_choice(String label, Common::Array<String> v) {
	uint n;

	g_vm->glk_window_clear(inputwin);

	glk_put_cstring(label.c_str());
	g_vm->glk_put_char(0x0a);
	n = v.size();
	for (uint i = 0; i < n; ++i) {
		StringStream t;
		String s;
		t << i + 1;
		t >> s;
		glk_put_cstring(s.c_str());
		glk_put_cstring(": ");
		glk_put_cstring(v[i].c_str());
		glk_put_cstring("\n");
	}

	StringStream t;
	String s;
	String s1;
	t << n;
	t >> s;
	s1 = "Choose [1-" + s + "]> ";
	g_vm->glk_put_string_stream(inputwinstream, s1.c_str());

	int choice = atoi(get_string().c_str());
	if (choice < 1) {
		choice = 1;
	}
	if ((uint)choice > n) {
		choice = n;
	}

	StringStream u;
	u << choice;
	u >> s;
	s1 = "Chosen: " +  s + "\n";
	glk_put_cstring(s1.c_str());

	return choice - 1;
}

String GeasGlkInterface::absolute_name(String rel_name, String parent) const {
	cerr << "absolute_name ('" << rel_name << "', '" << parent << "')\n";
	if (parent[0] != '/')
		return rel_name;

	if (rel_name[0] == '/') {
		cerr << "  --> " << rel_name << "\n";
		return rel_name;
	}
	Common::Array<String> path;
	uint dir_start = 1, dir_end;
	while (dir_start < parent.length()) {
		dir_end = dir_start;
		while (dir_end < parent.length() && parent[dir_end] != '/')
			dir_end ++;
		path.push_back(parent.substr(dir_start, dir_end - dir_start));
		dir_start = dir_end + 1;
	}
	path.pop_back();
	dir_start = 0;
	String tmp;
	while (dir_start < rel_name.length()) {
		dir_end = dir_start;
		while (dir_end < rel_name.length() && rel_name[dir_end] != '/')
			dir_end ++;
		tmp = rel_name.substr(dir_start, dir_end - dir_start);
		dir_start = dir_end + 1;
		if (tmp == ".")
			continue;
		else if (tmp == "..")
			path.pop_back();
		else
			path.push_back(tmp);
	}
	String rv;
	for (uint i = 0; i < path.size(); i ++)
		rv = rv + "/" + path[i];
	cerr << " ---> " << rv << "\n";
	return rv;
}

} // End of namespace Quest
} // End of namespace Glk
