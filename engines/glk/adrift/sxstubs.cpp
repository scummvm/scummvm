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

#include "glk/adrift/scare.h"
#include "glk/adrift/sxprotos.h"
#include "common/str.h"

namespace Glk {
namespace Adrift {

/*
 * Module notes:
 *
 * o Better, or perhaps less strace-like, tracing might be more beneficial
 *   for game script debugging.
 */

/* Stubs trace flag. */
static sc_bool stub_trace = FALSE;

/*
 * Input/output handler functions.  If assigned, calls to os_* functions are
 * routed here to allow the script runner to catch interpeter i/o.
 */
static sc_bool(*stub_read_line)(sc_char *, sc_int) = NULL;
static void (*stub_print_string)(const sc_char *) = NULL;
static void *(*stub_open_file)(sc_bool) = NULL;
static sc_int(*stub_read_file)(void *, sc_byte *, sc_int) = NULL;
static void (*stub_write_file)(void *, const sc_byte *, sc_int) = NULL;
static void (*stub_close_file)(void *) = NULL;

/* Flags for whether to report tags and resources via stub_print_string(). */
static sc_int stub_show_resources = 0;
static sc_int stub_show_tags = 0;


/*
 * stub_attach_handlers()
 * stub_detach_handlers()
 *
 * Attach input/output handler functions, and reset to NULLs.
 */
void
stub_attach_handlers(sc_bool(*read_line)(sc_char *, sc_int),
                     void (*print_string)(const sc_char *),
                     void *(*open_file)(sc_bool),
                     sc_int(*read_file)(void *, sc_byte *, sc_int),
                     void (*write_file)(void *, const sc_byte *, sc_int),
                     void (*close_file)(void *)) {
	stub_read_line = read_line;
	stub_print_string = print_string;
	stub_open_file = open_file;
	stub_read_file = read_file;
	stub_write_file = write_file;
	stub_close_file = close_file;

	stub_show_resources = 0;
	stub_show_tags = 0;
}

void
stub_detach_handlers(void) {
	stub_read_line = NULL;
	stub_print_string = NULL;
	stub_open_file = NULL;
	stub_read_file = NULL;
	stub_write_file = NULL;
	stub_close_file = NULL;

	stub_show_resources = 0;
	stub_show_tags = 0;
}


/*
 * stub_adjust_test_control()
 * stub_catch_test_control()
 *
 * Trap testing control tags from the game, incrementing or decrementing
 * stub_show_resources or stub_show_tags if a testing control tag arrives.
 * Returns TRUE if the tag trapped was one of our testing ones.
 */
static void
stub_adjust_test_control(sc_int *control, sc_bool is_begin) {
	*control += is_begin ? 1 : (*control > 0 ? -1 : 0);
}

static sc_bool
stub_catch_test_control(sc_int tag, const sc_char *argument) {
	if (tag == SC_TAG_UNKNOWN && argument) {
		sc_bool is_begin;
		const sc_char *name;

		is_begin = !(argument[0] == '/');
		name = is_begin ? argument : argument + 1;

		if (sc_strcasecmp(name, "sxshowresources") == 0) {
			stub_adjust_test_control(&stub_show_resources, is_begin);
			return TRUE;
		} else if (sc_strcasecmp(name, "sxshowtags") == 0) {
			stub_adjust_test_control(&stub_show_tags, is_begin);
			return TRUE;
		}
	}

	return FALSE;
}


/*
 * stub_notnull()
 *
 * Returns the string address passed in, or "(nil)" if NULL, for printf
 * safety.  Most libc's handle this themselves, but it's not defined by ANSI.
 */
static const sc_char *
stub_notnull(const sc_char *string) {
	return string ? string : "(nil)";
}


/*
 * os_*()
 *
 * Stub functions called by the interpreter core.
 */
void
os_print_tag(sc_int tag, const sc_char *argument) {
	if (stub_trace)
		sx_trace("os_print_tag (%ld, \"%s\")\n", tag, stub_notnull(argument));

	if (!stub_catch_test_control(tag, argument)) {
		if (stub_print_string) {
			if (stub_show_tags > 0) {
				stub_print_string("<<Tag: id=");
				Common::String buffer = Common::String::format("%ld", tag);
				stub_print_string(buffer.c_str());
				stub_print_string(", argument=\"");
				stub_print_string(stub_notnull(argument));
				stub_print_string("\">>");
			} else if (tag == SC_TAG_WAITKEY || tag == SC_TAG_CLS)
				stub_print_string(" ");
		}
	}
}

void
os_print_string(const sc_char *string) {
	if (stub_trace)
		sx_trace("os_print_string (\"%s\")\n", stub_notnull(string));

	if (stub_print_string)
		stub_print_string(string);
}

void
os_print_string_debug(const sc_char *string) {
	if (stub_trace)
		sx_trace("os_print_string_debug (\"%s\")\n", stub_notnull(string));

	if (stub_print_string)
		stub_print_string(string);
}

void
os_play_sound(const sc_char *filepath,
              sc_int offset, sc_int length, sc_bool is_looping) {
	if (stub_trace)
		sx_trace("os_play_sound (\"%s\", %ld, %ld, %s)\n",
		         stub_notnull(filepath), offset, length,
		         is_looping ? "true" : "false");

	if (stub_print_string && stub_show_resources > 0) {
		sc_char buffer[32];

		stub_print_string("<<Sound: id=\"");
		stub_print_string(stub_notnull(filepath));
		stub_print_string("\", offset=");
		sprintf(buffer, "%ld", offset);
		stub_print_string(buffer);
		stub_print_string(", length=");
		sprintf(buffer, "%ld", length);
		stub_print_string(buffer);
		stub_print_string(", looping=");
		stub_print_string(is_looping ? "true" : "false");
		stub_print_string(">>");
	}
}

void
os_stop_sound(void) {
	if (stub_trace)
		sx_trace("os_stop_sound ()\n");

	if (stub_print_string && stub_show_resources > 0)
		stub_print_string("<<Sound: stop>>");
}

void
os_show_graphic(const sc_char *filepath, sc_int offset, sc_int length) {
	if (stub_trace)
		sx_trace("os_show_graphic (\"%s\", %ld, %ld)\n",
		         stub_notnull(filepath), offset, length);

	if (stub_print_string && stub_show_resources > 0) {
		sc_char buffer[32];

		stub_print_string("<<Graphic: id=\"");
		stub_print_string(stub_notnull(filepath));
		stub_print_string("\", offset=");
		sprintf(buffer, "%ld", offset);
		stub_print_string(buffer);
		stub_print_string(", length=");
		sprintf(buffer, "%ld", length);
		stub_print_string(buffer);
		stub_print_string(">>");
	}
}

sc_bool
os_read_line(sc_char *buffer, sc_int length) {
	sc_bool status;

	if (stub_read_line)
		status = stub_read_line(buffer, length);
	else {
		assert(buffer && length > 4);
		sprintf(buffer, "%s", "quit");
		status = TRUE;
	}

	if (stub_trace) {
		if (status)
			sx_trace("os_read_line (\"%s\", %ld) -> true\n",
			         stub_notnull(buffer), length);
		else
			sx_trace("os_read_line (\"...\", %ld) -> false\n", length);
	}
	return status;
}

sc_bool
os_read_line_debug(sc_char *buffer, sc_int length) {
	assert(buffer && length > 8);
	sprintf(buffer, "%s", "continue");

	if (stub_trace)
		sx_trace("os_read_line_debug (\"%s\", %ld) -> true\n", buffer, length);
	return TRUE;
}

sc_bool
os_confirm(sc_int type) {
	if (stub_trace)
		sx_trace("os_confirm (%ld) -> true\n", type);
	return TRUE;
}

void *
os_open_file(sc_bool is_save) {
	void *opaque;

	if (stub_open_file)
		opaque = stub_open_file(is_save);
	else
		opaque = NULL;

	if (stub_trace) {
		if (opaque)
			sx_trace("os_open_file (%s) -> %p\n",
			         is_save ? "true" : "false", opaque);
		else
			sx_trace("os_open_file (%s) -> null\n", is_save ? "true" : "false");
	}
	return opaque;
}

sc_int
os_read_file(void *opaque, sc_byte *buffer, sc_int length) {
	sc_int bytes;

	if (stub_read_file)
		bytes = stub_read_file(opaque, buffer, length);
	else
		bytes = 0;

	if (stub_trace)
		sx_trace("os_read_file (%p, %p, %ld) -> %ld\n",
		         opaque, buffer, length, bytes);
	return bytes;
}

void
os_write_file(void *opaque, const sc_byte *buffer, sc_int length) {
	if (stub_write_file)
		stub_write_file(opaque, buffer, length);

	if (stub_trace)
		sx_trace("os_write_file (%p, %p, %ld)\n", opaque, buffer, length);
}

void
os_close_file(void *opaque) {
	if (stub_close_file)
		stub_close_file(opaque);

	if (stub_trace)
		sx_trace("os_close_file (%p)\n", opaque);
}

void
os_display_hints(sc_game game) {
	if (stub_trace)
		sx_trace("os_display_hints (%p)\n", game);

	if (stub_print_string) {
		sc_game_hint hint;

		for (hint = sc_get_first_game_hint(game);
		        hint; hint = sc_get_next_game_hint(game, hint)) {
			const sc_char *hint_text;

			stub_print_string(sc_get_game_hint_question(game, hint));
			stub_print_string("\n");

			hint_text = sc_get_game_subtle_hint(game, hint);
			if (hint_text) {
				stub_print_string("- ");
				stub_print_string(hint_text);
				stub_print_string("\n");
			}

			hint_text = sc_get_game_unsubtle_hint(game, hint);
			if (hint_text) {
				stub_print_string("- ");
				stub_print_string(hint_text);
				stub_print_string("\n");
			}
		}
	}
}


/*
 * stub_debug_trace()
 *
 * Set stubs tracing on/off.
 */
void
stub_debug_trace(sc_bool flag) {
	stub_trace = flag;
}

} // End of namespace Adrift
} // End of namespace Glk
