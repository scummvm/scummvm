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

#include "glk/adrift/adrift.h"
#include "glk/adrift/scprotos.h"
#include "glk/adrift/scgamest.h"
#include "common/file.h"
#include "common/system.h"
#include "common/savefile.h"

namespace Glk {
namespace Adrift {

/* Assorted definitions and constants. */
static const sc_char NEWLINE = '\n';
static const sc_char CARRIAGE_RETURN = '\r';
static const sc_char NUL = '\0';

/* Global tracing flags. */
static sc_uint if_trace_flags = 0;


/*
 * if_initialize()
 *
 * First-time runtime checks for the overall interpreter.  This function
 * tries to ensure correct compile options.
 */
static void if_initialize(void) {
	static sc_bool initialized = FALSE;

	/* Only do checks on the first call. */
	if (!initialized) {
		/* Make a few quick checks on types and type sizes. */
		if (sizeof(sc_byte) != 1 || sizeof(sc_char) != 1) {
			sc_error("if_initialize: sizeof sc_byte or sc_char"
			         " is not 1, check compile options\n");
		} else if (sizeof(sc_uint) < 4 || sizeof(sc_int) < 4) {
			sc_error("if_initialize: sizeof sc_uint or sc_int"
			         " is not at least 4, check compile options\n");
		} else if (sizeof(sc_uint) > 8 || sizeof(sc_int) > 8) {
			sc_error("if_initialize: sizeof sc_uint or sc_int"
			         " is more than 8, check compile options\n");
		} else if (!((sc_uint) - 1 > 0)) {
			sc_error("if_initialize: sc_uint appears not to be unsigned,"
			         " check compile options\n");
		}

		initialized = TRUE;
	}
}


/*
 * if_bool()
 * sc_set_trace_flags()
 * if_get_trace_flag()
 *
 * Set and retrieve tracing flags.  Setting new values propagates the new
 * tracing setting to all modules that support it.
 */
static sc_bool if_bool(sc_uint flag) {
	return flag ? TRUE : FALSE;
}

void sc_set_trace_flags(sc_uint trace_flags) {
	if_initialize();

	/* Save the value for queries. */
	if_trace_flags = trace_flags;

	/* Propagate tracing to modules that support it. */
	parse_debug_trace(if_bool(trace_flags & SC_TRACE_PARSE));
	prop_debug_trace(if_bool(trace_flags & SC_TRACE_PROPERTIES));
	var_debug_trace(if_bool(trace_flags & SC_TRACE_VARIABLES));
	uip_debug_trace(if_bool(trace_flags & SC_TRACE_PARSER));
	lib_debug_trace(if_bool(trace_flags & SC_TRACE_LIBRARY));
	evt_debug_trace(if_bool(trace_flags & SC_TRACE_EVENTS));
	npc_debug_trace(if_bool(trace_flags & SC_TRACE_NPCS));
	obj_debug_trace(if_bool(trace_flags & SC_TRACE_OBJECTS));
	task_debug_trace(if_bool(trace_flags & SC_TRACE_TASKS));
	restr_debug_trace(if_bool(trace_flags & SC_TRACE_TASKS));
	pf_debug_trace(if_bool(trace_flags & SC_TRACE_PRINTFILTER));
}

sc_bool if_get_trace_flag(sc_uint bitmask) {
	return if_bool(if_trace_flags & bitmask);
}


/*
 * if_print_string_common()
 * if_print_string()
 * if_print_debug()
 * if_print_character_common()
 * if_print_character()
 * if_print_debug_character()
 * if_print_tag()
 *
 * Call OS-specific print function for the given arguments.
 */
static void if_print_string_common(const sc_char *string, void (*print_string_function)(const sc_char *)) {
	assert(string);

	if (string[0] != NUL)
		print_string_function(string);
}

void if_print_string(const sc_char *string) {
	if_print_string_common(string, os_print_string);
}

void if_print_debug(const sc_char *string) {
	if_print_string_common(string, os_print_string_debug);
}

static void if_print_character_common(sc_char character, void (*print_string_function)(const sc_char *)) {
	if (character != NUL) {
		sc_char buffer[2];

		buffer[0] = character;
		buffer[1] = NUL;
		print_string_function(buffer);
	}
}

void if_print_character(sc_char character) {
	if_print_character_common(character, os_print_string);
}

void if_print_debug_character(sc_char character) {
	if_print_character_common(character, os_print_string_debug);
}

void if_print_tag(sc_int tag, const sc_char *arg) {
	assert(arg);

	os_print_tag(tag, arg);
}


/*
 * if_read_line_common()
 * if_read_line()
 * if_read_debug()
 *
 * Call OS-specific line read function.  Clean up any read data a little
 * before returning it to the caller.
 */
static void if_read_line_common(sc_char *buffer, sc_int length,
		sc_bool(*read_line_function)(sc_char *, sc_int)) {
	sc_bool is_line_available;
	sc_int last;
	assert(buffer && length > 0);

	/* Loop until valid player input is available. */
	do {
		/* Space first with a blank line, and clear the buffer. */
		if_print_character('\n');
		memset(buffer, NUL, length);

		is_line_available = read_line_function(buffer, length);
		if (g_vm->shouldQuit())
			return;
	} while (!is_line_available);

	/* Drop any trailing newline/return. */
	last = strlen(buffer) - 1;
	while (last >= 0
	        && (buffer[last] == CARRIAGE_RETURN || buffer[last] == NEWLINE))
		buffer[last--] = NUL;
}

void if_read_line(sc_char *buffer, sc_int length) {
	if_read_line_common(buffer, length, os_read_line);
}

void if_read_debug(sc_char *buffer, sc_int length) {
	if_read_line_common(buffer, length, os_read_line_debug);
}


/*
 * if_confirm()
 *
 * Call OS-specific confirm function.
 */
sc_bool if_confirm(sc_int type) {
	return os_confirm(type);
}


/*
 * if_open_saved_game()
 * if_write_saved_game()
 * if_read_saved_game()
 * if_close_saved_game()
 *
 * Call OS-specific functions for saving and restoring games.
 */
void *if_open_saved_game(sc_bool is_save) {
	return os_open_file(is_save);
}

void if_write_saved_game(void *opaque, const sc_byte *buffer, sc_int length) {
	assert(buffer);

	os_write_file(opaque, buffer, length);
}

sc_int if_read_saved_game(void *opaque, sc_byte *buffer, sc_int length) {
	assert(buffer);

	return os_read_file(opaque, buffer, length);
}

void if_close_saved_game(void *opaque) {
	os_close_file(opaque);
}


/*
 * if_display_hints()
 *
 * Call OS-specific hint display function.
 */
void if_display_hints(sc_gameref_t game) {
	assert(gs_is_game_valid(game));

	os_display_hints((sc_game) game);
}


/*
 * if_update_sound()
 * if_update_graphic()
 *
 * Call OS-specific sound and graphic handler functions.
 */
void if_update_sound(const sc_char *filename, sc_int sound_offset, sc_int sound_length,
		sc_bool is_looping) {
	if (strlen(filename) > 0)
		os_play_sound(filename, sound_offset, sound_length, is_looping);
	else
		os_stop_sound();
}

void if_update_graphic(const sc_char *filename,
                  sc_int graphic_offset, sc_int graphic_length) {
	os_show_graphic(filename, graphic_offset, graphic_length);
}


/*
 * sc_scare_version()
 * sc_scare_emulation()
 *
 * Return a version string and Adrift emulation level.
 */
const sc_char *sc_scare_version(void) {
	if_initialize();
	return "SCARE " SCARE_VERSION SCARE_PATCH_LEVEL;
}

sc_int sc_scare_emulation(void) {
	if_initialize();
	return SCARE_EMULATION;
}


/*
 * if_file_read_callback()
 * if_file_write_callback()
 *
 * Standard FILE* reader and writer callback for constructing callback-style
 * calls from filename and stream variants.
 */
static sc_int if_file_read_callback(void *opaque, sc_byte *buffer, sc_int length) {
	Common::SeekableReadStream *stream = (Common::SeekableReadStream *)opaque;
	sc_int bytes;

	bytes = stream->read(buffer, length);
	if (stream->err())
		sc_error("if_file_read_callback: warning: read error\n");

	return bytes;
}

static void if_file_write_callback(void *opaque, const sc_byte *buffer, sc_int length) {
	Common::WriteStream *stream = (Common::WriteStream *) opaque;

	stream->write(buffer, length);
	if (stream->err())
		sc_error("if_file_write_callback: warning: write error\n");
}


/*
 * sc_game_from_filename()
 * sc_game_from_stream()
 * sc_game_from_callback()
 *
 * Called by the OS-specific layer to create a run context.  The _filename()
 * and _stream() variants are adapters for run_create().
 */
sc_game sc_game_from_filename(const sc_char *filename) {
	Common::File *stream;
	sc_game game;

	if_initialize();
	if (!filename) {
		sc_error("sc_game_from_filename: nullptr filename\n");
		return nullptr;
	}

	stream = new Common::File();
	if (!stream->open(filename)) {
		delete stream;
		sc_error("sc_game_from_filename: fopen error\n");
		return nullptr;
	}

	game = run_create(if_file_read_callback, stream);
	delete stream;

	return game;
}

sc_game sc_game_from_stream(Common::SeekableReadStream *stream) {
	if_initialize();
	if (!stream) {
		sc_error("sc_game_from_stream: nullptr stream\n");
		return nullptr;
	}

	return run_create(if_file_read_callback, stream);
}

sc_game sc_game_from_callback(sc_int(*callback)(void *, sc_byte *, sc_int), void *opaque) {
	if_initialize();
	if (!callback) {
		sc_error("sc_game_from_callback: nullptr callback\n");
		return nullptr;
	}

	return run_create(callback, opaque);
}


/*
 * if_game_error()
 *
 * Common function to verify that the game passed in to functions below
 * is a valid game.  Returns TRUE on game error, FALSE if okay.
 */
static sc_bool if_game_error(const sc_gameref_t game, const sc_char *function_name) {
	/* Check for invalid game -- null pointer or bad magic. */
	if (!gs_is_game_valid(game)) {
		if (game)
			sc_error("%s: invalid game\n", function_name);
		else
			sc_error("%s: nullptr game\n", function_name);
		return TRUE;
	}

	/* No game error. */
	return FALSE;
}


/*
 * sc_interpret_game()
 * sc_restart_game()
 * sc_save_game()
 * sc_load_game()
 * sc_undo_game_turn()
 * sc_quit_game()
 *
 * Called by the OS-specific layer to run a game loaded into a run context,
 * and to quit the interpreter on demand, if required.  sc_quit_game()
 * is implemented as a longjmp(), so never returns to the caller --
 * instead, the program behaves as if sc_interpret_game() had returned.
 * sc_load_game() will longjmp() if the restore is successful (thus
 * behaving like sc_restart_game()), but will return if the game could not
 * be restored.  sc_undo_game_turn() behaves like sc_load_game().
 */
void sc_interpret_game(CONTEXT, sc_game game) {
	const sc_gameref_t game_ = (sc_gameref_t)game;

	if (if_game_error(game_, "sc_interpret_game"))
		return;

	run_interpret(context, game_);
}

void sc_restart_game(CONTEXT, sc_game game) {
	const sc_gameref_t game_ = (sc_gameref_t)game;

	if (if_game_error(game_, "sc_restart_game"))
		return;

	CALL1(run_restart, game_);
}

sc_bool sc_save_game(sc_game game) {
	const sc_gameref_t game_ = (sc_gameref_t)game;

	if (if_game_error(game_, "sc_save_game"))
		return FALSE;

	return run_save_prompted(game_);
}

sc_bool sc_load_game(CONTEXT, sc_game game) {
	const sc_gameref_t game_ = (sc_gameref_t)game;

	if (if_game_error(game_, "sc_load_game"))
		return FALSE;

	return run_restore_prompted(context, game_);
}

sc_bool sc_undo_game_turn(CONTEXT, sc_game game) {
	const sc_gameref_t game_ = (sc_gameref_t)game;

	if (if_game_error(game_, "sc_undo_game_turn"))
		return FALSE;

	return run_undo(context, game_);
}

void sc_quit_game(CONTEXT, sc_game game) {
	const sc_gameref_t game_ = (sc_gameref_t)game;

	if (if_game_error(game_, "sc_quit_game"))
		return;

	run_quit(context, game_);
}


/*
 * sc_save_game_to_filename()
 * sc_save_game_to_stream()
 * sc_save_game_to_callback()
 * sc_load_game_from_filename()
 * sc_load_game_from_stream()
 * sc_load_game_from_callback()
 *
 * Low level game saving and loading functions.  The normal sc_save_game()
 * and sc_load_game() functions act exactly as the "save" and "restore"
 * game commands, in that they prompt the user for a stream to write or read.
 * These alternative forms allow the caller to directly specify the data
 * streams.
 */
sc_bool sc_save_game_to_filename(sc_game game, const sc_char *filename) {
	const sc_gameref_t game_ = (sc_gameref_t)game;
	Common::OutSaveFile *sf;

	if (if_game_error(game_, "sc_save_game_to_filename"))
		return FALSE;

	if (!filename) {
		sc_error("sc_save_game_to_filename: nullptr filename\n");
		return FALSE;
	}

	sf = g_system->getSavefileManager()->openForSaving(filename);
	if (!sf) {
		sc_error("sc_save_game_to_filename: fopen error\n");
		return FALSE;
	}

	run_save(game_, if_file_write_callback, sf);
	sf->finalize();
	delete sf;

	return TRUE;
}

void sc_save_game_to_stream(sc_game game, Common::SeekableReadStream *stream) {
	const sc_gameref_t game_ = (sc_gameref_t)game;

	if (if_game_error(game_, "sc_save_game_to_stream"))
		return;

	if (!stream) {
		sc_error("sc_save_game_to_stream: nullptr stream\n");
		return;
	}

	run_save(game_, if_file_write_callback, stream);
}

void sc_save_game_to_callback(sc_game game,
		void (*callback)(void *, const sc_byte *, sc_int), void *opaque) {
	const sc_gameref_t game_ = (sc_gameref_t)game;

	if (if_game_error(game_, "sc_save_game_to_callback"))
		return;

	if (!callback) {
		sc_error("sc_save_game_to_callback: nullptr callback\n");
		return;
	}

	run_save(game_, callback, opaque);
}

sc_bool sc_load_game_from_filename(CONTEXT, sc_game game, const sc_char *filename) {
	const sc_gameref_t game_ = (sc_gameref_t)game;
	Common::InSaveFile *sf;
	sc_bool status;

	if (if_game_error(game_, "sc_load_game_from_filename"))
		return FALSE;

	if (!filename) {
		sc_error("sc_load_game_from_filename: nullptr filename\n");
		return FALSE;
	}

	sf = g_system->getSavefileManager()->openForLoading(filename);
	if (!sf) {
		sc_error("sc_load_game_from_filename: fopen error\n");
		return FALSE;
	}

	status = run_restore(context, game_, if_file_read_callback, sf);
	delete sf;

	return status;
}

sc_bool sc_load_game_from_stream(CONTEXT, sc_game game, Common::SeekableReadStream *stream) {
	const sc_gameref_t game_ = (sc_gameref_t)game;

	if (if_game_error(game_, "sc_load_game_from_stream"))
		return FALSE;

	if (!stream) {
		sc_error("sc_load_game_from_stream: nullptr stream\n");
		return FALSE;
	}

	return run_restore(context, game_, if_file_read_callback, stream);
}

sc_bool sc_load_game_from_callback(CONTEXT, sc_game game,
		sc_int(*callback)(void *, sc_byte *, sc_int), void *opaque) {
	const sc_gameref_t game_ = (sc_gameref_t)game;

	if (if_game_error(game_, "sc_load_game_from_callback"))
		return FALSE;

	if (!callback) {
		sc_error("sc_load_game_from_callback: nullptr callback\n");
		return FALSE;
	}

	return run_restore(context, game_, callback, opaque);
}


/*
 * sc_free_game()
 *
 * Called by the OS-specific layer to free run context memory.
 */
void sc_free_game(sc_game game) {
	const sc_gameref_t game_ = (sc_gameref_t)game;

	if (if_game_error(game_, "sc_free_game"))
		return;

	run_destroy(game_);
}


/*
 * sc_is_game_running()
 * sc_get_game_name()
 * sc_get_game_author()
 * sc_get_game_compile_date()
 * sc_get_game_turns()
 * sc_get_game_score()
 * sc_get_game_max_score()
 * sc_get_game_room ()
 * sc_get_game_status_line ()
 * sc_get_game_preferred_font ()
 * sc_get_game_bold_room_names()
 * sc_get_game_verbose()
 * sc_get_game_notify_score_change()
 * sc_has_game_completed()
 * sc_is_game_undo_available()
 *
 * Return a few attributes of a game.
 */
sc_bool sc_is_game_running(sc_game game) {
	const sc_gameref_t game_ = (sc_gameref_t)game;

	if (if_game_error(game_, "sc_is_game_running"))
		return FALSE;

	return run_is_running(game_);
}

const sc_char *sc_get_game_name(sc_game game) {
	const sc_gameref_t game_ = (sc_gameref_t)game;
	const sc_char *retval;

	if (if_game_error(game_, "sc_get_game_name"))
		return "[invalid game]";

	run_get_attributes(game_, &retval,
	                   nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	                   nullptr, nullptr);
	return retval;
}

const sc_char *
sc_get_game_author(sc_game game) {
	const sc_gameref_t game_ = (sc_gameref_t)game;
	const sc_char *retval;

	if (if_game_error(game_, "sc_get_game_author"))
		return "[invalid game]";

	run_get_attributes(game_, nullptr, &retval,
	                   nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	                   nullptr);
	return retval;
}

const sc_char *sc_get_game_compile_date(sc_game game) {
	const sc_gameref_t game_ = (sc_gameref_t)game;
	const sc_char *retval;

	if (if_game_error(game_, "sc_get_game_compile_date"))
		return "[invalid game]";

	run_get_attributes(game_, nullptr, nullptr, &retval,
	                   nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
	return retval;
}

sc_int sc_get_game_turns(sc_game game) {
	const sc_gameref_t game_ = (sc_gameref_t)game;
	sc_int retval;

	if (if_game_error(game_, "sc_get_game_turns"))
		return 0;

	run_get_attributes(game_, nullptr, nullptr, nullptr, &retval,
	                   nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
	return retval;
}

sc_int sc_get_game_score(sc_game game) {
	const sc_gameref_t game_ = (sc_gameref_t)game;
	sc_int retval;

	if (if_game_error(game_, "sc_get_game_score"))
		return 0;

	run_get_attributes(game_, nullptr, nullptr, nullptr, nullptr, &retval,
	                   nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
	return retval;
}

sc_int sc_get_game_max_score(sc_game game) {
	const sc_gameref_t game_ = (sc_gameref_t)game;
	sc_int retval;

	if (if_game_error(game_, "sc_get_game_max_score"))
		return 0;

	run_get_attributes(game_, nullptr, nullptr, nullptr, nullptr, nullptr, &retval,
	                   nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
	return retval;
}

const sc_char *sc_get_game_room(sc_game game) {
	const sc_gameref_t game_ = (sc_gameref_t)game;
	const sc_char *retval;

	if (if_game_error(game_, "sc_get_game_room"))
		return "[invalid game]";

	run_get_attributes(game_, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, &retval,
	                   nullptr, nullptr, nullptr, nullptr, nullptr);
	return retval;
}

const sc_char *sc_get_game_status_line(sc_game game) {
	const sc_gameref_t game_ = (sc_gameref_t)game;
	const sc_char *retval;

	if (if_game_error(game_, "sc_get_game_status_line"))
		return "[invalid game]";

	run_get_attributes(game_, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	                   &retval, nullptr, nullptr, nullptr, nullptr);
	return retval;
}

const sc_char *sc_get_game_preferred_font(sc_game game) {
	const sc_gameref_t game_ = (sc_gameref_t)game;
	const sc_char *retval;

	if (if_game_error(game_, "sc_get_game_preferred_font"))
		return "[invalid game]";

	run_get_attributes(game_, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	                   nullptr, &retval, nullptr, nullptr, nullptr);
	return retval;
}

sc_bool sc_get_game_bold_room_names(sc_game game) {
	const sc_gameref_t game_ = (sc_gameref_t)game;
	sc_bool retval;

	if (if_game_error(game_, "sc_get_game_bold_room_names"))
		return FALSE;

	run_get_attributes(game_, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	                   nullptr, nullptr, &retval, nullptr, nullptr);
	return retval;
}

sc_bool sc_get_game_verbose(sc_game game) {
	const sc_gameref_t game_ = (sc_gameref_t)game;
	sc_bool retval;

	if (if_game_error(game_, "sc_get_game_verbose"))
		return FALSE;

	run_get_attributes(game_, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	                   nullptr, nullptr, nullptr, &retval, nullptr);
	return retval;
}

sc_bool sc_get_game_notify_score_change(sc_game game) {
	const sc_gameref_t game_ = (sc_gameref_t)game;
	sc_bool retval;

	if (if_game_error(game_, "sc_get_game_notify_score_change"))
		return FALSE;

	run_get_attributes(game_, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	                   nullptr, nullptr, nullptr, nullptr, &retval);
	return retval;
}

sc_bool sc_has_game_completed(sc_game game) {
	const sc_gameref_t game_ = (sc_gameref_t)game;

	if (if_game_error(game_, "sc_has_game_completed"))
		return FALSE;

	return run_has_completed(game_);
}

sc_bool sc_is_game_undo_available(sc_game game) {
	const sc_gameref_t game_ = (sc_gameref_t)game;

	if (if_game_error(game_, "sc_is_game_undo_available"))
		return FALSE;

	return run_is_undo_available(game_);
}


/*
 * sc_set_game_bold_room_names()
 * sc_set_game_verbose()
 * sc_set_game_notify_score_change()
 *
 * Set a few attributes of a game.
 */
void sc_set_game_bold_room_names(sc_game game, sc_bool flag) {
	const sc_gameref_t game_ = (sc_gameref_t)game;
	sc_bool bold, verbose, notify;

	if (if_game_error(game_, "sc_set_game_bold_room_names"))
		return;

	run_get_attributes(game_, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	                   nullptr, nullptr, &bold, &verbose, &notify);
	run_set_attributes(game_, flag, verbose, notify);
}

void sc_set_game_verbose(sc_game game, sc_bool flag) {
	const sc_gameref_t game_ = (sc_gameref_t)game;
	sc_bool bold, verbose, notify;

	if (if_game_error(game_, "sc_set_game_verbose"))
		return;

	run_get_attributes(game_, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	                   nullptr, nullptr, &bold, &verbose, &notify);
	run_set_attributes(game_, bold, flag, notify);
}

void sc_set_game_notify_score_change(sc_game game, sc_bool flag) {
	const sc_gameref_t game_ = (sc_gameref_t)game;
	sc_bool bold, verbose, notify;

	if (if_game_error(game_, "sc_set_game_notify_score_change"))
		return;

	run_get_attributes(game_, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	                   nullptr, nullptr, &bold, &verbose, &notify);
	run_set_attributes(game_, bold, verbose, flag);
}


/*
 * sc_does_game_use_sounds()
 * sc_does_game_use_graphics()
 *
 * Indicate the game's use of resources.
 */
sc_bool sc_does_game_use_sounds(sc_game game) {
	const sc_gameref_t game_ = (sc_gameref_t)game;

	if (if_game_error(game_, "sc_does_game_use_sounds"))
		return FALSE;

	return res_has_sound(game_);
}

sc_bool sc_does_game_use_graphics(sc_game game) {
	const sc_gameref_t game_ = (sc_gameref_t)game;

	if (if_game_error(game_, "sc_does_game_use_graphics"))
		return FALSE;

	return res_has_graphics(game_);
}


/*
 * sc_get_first_game_hint()
 * sc_get_next_game_hint()
 * sc_get_game_hint_question()
 * sc_get_game_subtle_hint()
 * sc_get_game_sledgehammer_hint()
 *
 * Iterate currently available hints, and return strings for a hint.
 */
sc_game_hint sc_get_first_game_hint(sc_game game) {
	const sc_gameref_t game_ = (sc_gameref_t)game;

	if (if_game_error(game_, "sc_get_first_game_hint"))
		return nullptr;

	return run_hint_iterate(game_, nullptr);
}

sc_game_hint sc_get_next_game_hint(sc_game game, sc_game_hint hint) {
	const sc_gameref_t game_ = (sc_gameref_t)game;
	const sc_hintref_t hint_ = (sc_hintref_t)hint;

	if (if_game_error(game_, "sc_get_next_game_hint"))
		return nullptr;
	if (!hint_) {
		sc_error("sc_get_next_game_hint: nullptr hint\n");
		return nullptr;
	}

	return run_hint_iterate(game_, hint_);
}

const sc_char *sc_get_game_hint_question(sc_game game, sc_game_hint hint) {
	const sc_gameref_t game_ = (sc_gameref_t)game;
	const sc_hintref_t hint_ = (sc_hintref_t)hint;

	if (if_game_error(game_, "sc_get_game_hint_question"))
		return nullptr;
	if (!hint_) {
		sc_error("sc_get_game_hint_question: nullptr hint\n");
		return nullptr;
	}

	return run_get_hint_question(game_, hint_);
}

const sc_char *sc_get_game_subtle_hint(sc_game game, sc_game_hint hint) {
	const sc_gameref_t game_ = (sc_gameref_t)game;
	const sc_hintref_t hint_ = (sc_hintref_t)hint;

	if (if_game_error(game_, "sc_get_game_subtle_hint"))
		return nullptr;
	if (!hint_) {
		sc_error("sc_get_game_subtle_hint: nullptr hint\n");
		return nullptr;
	}

	return run_get_subtle_hint(game_, hint_);
}

const sc_char *sc_get_game_unsubtle_hint(sc_game game, sc_game_hint hint) {
	const sc_gameref_t game_ = (sc_gameref_t)game;
	const sc_hintref_t hint_ = (sc_hintref_t)hint;

	if (if_game_error(game_, "sc_get_game_unsubtle_hint"))
		return nullptr;
	if (!hint_) {
		sc_error("sc_get_game_unsubtle_hint: nullptr hint\n");
		return nullptr;
	}

	return run_get_unsubtle_hint(game_, hint_);
}


/*
 * sc_set_game_debugger_enabled()
 * sc_is_game_debugger_enabled()
 * sc_run_game_debugger_command()
 *
 * Enable, disable, and query game debugging, and run a single debug command.
 */
void sc_set_game_debugger_enabled(sc_game game, sc_bool flag) {
	const sc_gameref_t game_ = (sc_gameref_t)game;

	if (if_game_error(game_, "sc_set_game_debugger_enabled"))
		return;

	debug_set_enabled(game_, flag);
}

sc_bool sc_get_game_debugger_enabled(sc_game game) {
	const sc_gameref_t game_ = (sc_gameref_t)game;

	if (if_game_error(game_, "sc_get_game_debugger_enabled"))
		return FALSE;

	return debug_get_enabled(game_);
}

sc_bool sc_run_game_debugger_command(sc_game game, const sc_char *debug_command) {
	const sc_gameref_t game_ = (sc_gameref_t)game;

	if (if_game_error(game_, "sc_run_game_debugger_command"))
		return FALSE;

	return debug_run_command(game_, debug_command);
}


/*
 * sc_set_locale()
 * sc_get_locale()
 *
 * Set the interpreter locale, and get the currently set locale.
 */
sc_bool sc_set_locale(const sc_char *name) {
	if (!name) {
		sc_error("sc_set_locale: nullptr name\n");
		return FALSE;
	}

	return loc_set_locale(name);
}

const sc_char *sc_get_locale(void) {
	return loc_get_locale();
}


/*
 * sc_set_portable_random()
 * sc_reseed_random_sequence()
 *
 * Turn portable random number generation on and off, and supply a new seed
 * for random number generators.
 */
void sc_set_portable_random(sc_bool flag) {
	if (flag)
		sc_set_congruential_random();
	else
		sc_set_platform_random();
}

void sc_reseed_random_sequence(sc_uint new_seed) {
	if (new_seed == 0) {
		sc_error("sc_reseed_random_sequence: new_seed may not be 0\n");
		return;
	}

	sc_seed_random(new_seed);
}

} // End of namespace Adrift
} // End of namespace Glk
