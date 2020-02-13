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

#ifndef ADRIFT_ADRIFT_H
#define ADRIFT_ADRIFT_H

#include "common/scummsys.h"
#include "common/stream.h"
#include "glk/jumps.h"

namespace Glk {
namespace Adrift {

/*
 * Base type definitions.  SCARE integer types need to be at least 32 bits,
 * so using long here is a good bet for almost all ANSI C implementations for
 * 32 and 64 bit platforms; maybe also for any 16 bit ones.  For 64 bit
 * platforms configured for LP64, SCARE integer types will consume more space
 * in data structures.  Values won't wrap identically to 32 bit ones, but
 * games shouldn't be relying on wrapping anyway.  One final note -- in several
 * places, SCARE allocates 32 bytes into which it will sprintf() a long; this
 * is fine for both standard 32 bit and LP64 64 bit platforms, but is unsafe
 * should SCARE ever be configured for 128 bit definitions of sc_[u]int.
 */
typedef char sc_char;
typedef unsigned char sc_byte;
typedef long sc_int;
typedef unsigned long sc_uint;
typedef int sc_bool;

enum { BYTE_MAX = 0xff };
enum { INTEGER_MAX = 0x7fff };

/* Enumerated confirmation types, passed to os_confirm(). */
enum {
	SC_CONF_QUIT = 0,
	SC_CONF_RESTART, SC_CONF_SAVE, SC_CONF_RESTORE, SC_CONF_VIEW_HINTS
};

/* HTML-like tag enumerated values, passed to os_print_tag(). */
enum {
	SC_TAG_UNKNOWN = 0, SC_TAG_ITALICS, SC_TAG_ENDITALICS, SC_TAG_BOLD,
	SC_TAG_ENDBOLD, SC_TAG_UNDERLINE, SC_TAG_ENDUNDERLINE, SC_TAG_COLOR,
	SC_TAG_ENDCOLOR, SC_TAG_FONT, SC_TAG_ENDFONT, SC_TAG_BGCOLOR, SC_TAG_CENTER,
	SC_TAG_ENDCENTER, SC_TAG_RIGHT, SC_TAG_ENDRIGHT, SC_TAG_WAIT, SC_TAG_WAITKEY,
	SC_TAG_CLS,

	/* British spelling equivalents. */
	SC_TAG_COLOUR = SC_TAG_COLOR,
	SC_TAG_ENDCOLOUR = SC_TAG_ENDCOLOR,
	SC_TAG_BGCOLOUR = SC_TAG_BGCOLOR,
	SC_TAG_CENTRE = SC_TAG_CENTER,
	SC_TAG_ENDCENTRE = SC_TAG_ENDCENTER
};

/* OS interface function prototypes; interpreters must define these. */
typedef void *sc_game;
extern void os_print_string(const sc_char *string);
extern void os_print_tag(sc_int tag, const sc_char *argument);
extern void os_play_sound(const sc_char *filepath,
                          sc_int offset, sc_int length, sc_bool is_looping);
extern void os_stop_sound();
extern void os_show_graphic(const sc_char *filepath,
                            sc_int offset, sc_int length);
extern sc_bool os_read_line(sc_char *buffer, sc_int length);
extern sc_bool os_confirm(sc_int type);
extern void *os_open_file(sc_bool is_save);
extern void os_write_file(void *opaque, const sc_byte *buffer, sc_int length);
extern sc_int os_read_file(void *opaque, sc_byte *buffer, sc_int length);
extern void os_close_file(void *opaque);
extern void os_display_hints(sc_game game);

extern void os_print_string_debug(const sc_char *string);
extern sc_bool os_read_line_debug(sc_char *buffer, sc_int length);

/* Interpreter trace flag bits, passed to sc_set_trace_flags(). */
enum {
	SC_TRACE_PARSE = 1, SC_TRACE_PROPERTIES = 2, SC_TRACE_VARIABLES = 4,
	SC_TRACE_PARSER = 8, SC_TRACE_LIBRARY = 16, SC_TRACE_EVENTS = 32,
	SC_TRACE_NPCS = 64, SC_TRACE_OBJECTS = 128, SC_TRACE_TASKS = 256,
	SC_TRACE_PRINTFILTER = 512,

	SC_DUMP_TAF = 1024, SC_DUMP_PROPERTIES = 2048, SC_DUMP_VARIABLES = 4096,
	SC_DUMP_PARSER_TREES = 8192, SC_DUMP_LOCALE_TABLES = 16384
};

/* Module-wide trace control function prototype. */
extern void sc_set_trace_flags(sc_uint trace_flags);

/* Interpreter interface function prototypes. */
extern sc_game sc_game_from_filename(const sc_char *filename);
extern sc_game sc_game_from_stream(Common::SeekableReadStream *stream);
extern sc_game sc_game_from_callback(sc_int(*callback)
                                     (void *, sc_byte *, sc_int),
                                     void *opaque);
extern void sc_interpret_game(CONTEXT, sc_game game);
extern void sc_restart_game(CONTEXT, sc_game game);
extern sc_bool sc_save_game(sc_game game);
extern sc_bool sc_load_game(sc_game game);
extern sc_bool sc_undo_game_turn(CONTEXT, sc_game game);
extern void sc_quit_game(sc_game game);
extern sc_bool sc_save_game_to_filename(sc_game game, const sc_char *filename);
extern void sc_save_game_to_stream(sc_game game, Common::SeekableReadStream *stream);
extern void sc_save_game_to_callback(sc_game game,
                                     void (*callback)
                                     (void *, const sc_byte *, sc_int),
                                     void *opaque);
extern sc_bool sc_load_game_from_filename(sc_game game,
        const sc_char *filename);
extern sc_bool sc_load_game_from_stream(sc_game game, Common::SeekableReadStream *stream);
extern sc_bool sc_load_game_from_callback(sc_game game,
        sc_int(*callback)
        (void *, sc_byte *, sc_int),
        void *opaque);
extern void sc_free_game(sc_game game);
extern sc_bool sc_is_game_running(sc_game game);
extern const sc_char *sc_get_game_name(sc_game game);
extern const sc_char *sc_get_game_author(sc_game game);
extern const sc_char *sc_get_game_compile_date(sc_game game);
extern sc_int sc_get_game_turns(sc_game game);
extern sc_int sc_get_game_score(sc_game game);
extern sc_int sc_get_game_max_score(sc_game game);
extern const sc_char *sc_get_game_room(sc_game game);
extern const sc_char *sc_get_game_status_line(sc_game game);
extern const sc_char *sc_get_game_preferred_font(sc_game game);
extern sc_bool sc_get_game_bold_room_names(sc_game game);
extern sc_bool sc_get_game_verbose(sc_game game);
extern sc_bool sc_get_game_notify_score_change(sc_game game);
extern sc_bool sc_has_game_completed(sc_game game);
extern sc_bool sc_is_game_undo_available(sc_game game);
extern void sc_set_game_bold_room_names(sc_game game, sc_bool flag);
extern void sc_set_game_verbose(sc_game game, sc_bool flag);
extern void sc_set_game_notify_score_change(sc_game game, sc_bool flag);

extern sc_bool sc_does_game_use_sounds(sc_game);
extern sc_bool sc_does_game_use_graphics(sc_game);

typedef void *sc_game_hint;
extern sc_game_hint sc_get_first_game_hint(sc_game game);
extern sc_game_hint sc_get_next_game_hint(sc_game game, sc_game_hint hint);
extern const sc_char *sc_get_game_hint_question(sc_game game,
        sc_game_hint hint);
extern const sc_char *sc_get_game_subtle_hint(sc_game game,
        sc_game_hint hint);
extern const sc_char *sc_get_game_unsubtle_hint(sc_game game,
        sc_game_hint hint);

extern void sc_set_game_debugger_enabled(sc_game game, sc_bool flag);
extern sc_bool sc_get_game_debugger_enabled(sc_game game);
extern sc_bool sc_run_game_debugger_command(sc_game game,
        const sc_char *debug_command);
extern void sc_set_portable_random(sc_bool flag);
extern void sc_reseed_random_sequence(sc_uint new_seed);

/* Locale control and query functions. */
extern sc_bool sc_set_locale(const sc_char *name);
extern const sc_char *sc_get_locale();

/* A few possibly useful utilities. */
extern sc_int sc_strncasecmp(const sc_char *s1, const sc_char *s2, sc_int n);
extern sc_int sc_strcasecmp(const sc_char *s1, const sc_char *s2);
extern const sc_char *sc_scare_version();
extern sc_int sc_scare_emulation();

extern char *adrift_fgets(char *buf, int max, Common::SeekableReadStream *s);

} // End of namespace Adrift
} // End of namespace Glk

#endif
