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

namespace Glk {
namespace Adrift {

/*
 * Module notes:
 *
 * o The script file format is as follows.  Lines beginning '#' are comments
 *   and empty lines are ignored, otherwise the file is composed of sections.
 *   The first section line is one that starts with either '>' or '~'.  This
 *   is the next command.  The following lines, up to the next '>' or '~'
 *   section start, are concatenated into the expectation for the command.
 *   Expectations are glob patterns.  Commands starting with '>' are sent to
 *   the game; those starting with '~' are sent to the SCARE debugger.  Before
 *   the game is running, debugger commands are valid.  The first non-debugger
 *   command starts the game running.  An empty debugger command ('~') that
 *   follows any introductory debugger commands both starts the game and sets
 *   an expectation for the game's introductory text.  After the game has
 *   completed (or quit), only debugger commands are valid; others are ignored.
 *
 * o The script file structure is intentionally simple, but might be too
 *   simple for some purposes.
 */

/* Assorted definitions and constants. */
static const sc_int LINE_BUFFER_SIZE = 256;
static const sc_char NUL = '\0';
static const sc_char SCRIPT_COMMENT = '#';
static const sc_char GAME_COMMAND = '>';
static const sc_char DEBUG_COMMAND = '~';

/* Verbosity, and references to the game and script being processed. */
static sc_bool scr_is_verbose = FALSE;
static sc_game scr_game = NULL;
static sx_script scr_script = NULL;

/* Script line number, and count of errors registered for the script. */
static sc_int scr_line_number = 0;
static sc_int scr_errors = 0;

/*
 * Current expected output, and game accumulated output, used by the
 * expectation checking function.
 */
static sc_char *scr_expectation = NULL;
static sc_char *scr_game_output = NULL;


/*
 * scr_set_verbose()
 *
 * Set error reporting for expectation errors detected in the script.
 */
void scr_set_verbose(sc_bool flag) {
	scr_is_verbose = flag;
}


/*
 * scr_test_message()
 * scr_test_failed()
 *
 * Simple common message and test case failure handling functions.  The second
 * is used by the serialization helper, so is not static.
 */
static void scr_test_message(const sc_char *format, const sc_char *string) {
	if (scr_is_verbose) {
		sx_trace("--- ");
		sx_trace(format, string);
		sx_trace("\n");
	}
}

void scr_test_failed(const sc_char *format, const sc_char *string) {
	assert(format && string);

	if (scr_is_verbose) {
		if (scr_line_number > 0)
			sx_trace("--- Near line %ld: ", scr_line_number);
		else
			sx_trace("--- ");
		sx_trace(format, string);
		sx_trace("\n");
	}
	scr_errors++;
}


/*
 * scr_is_line_type()
 * scr_is_line_comment_or_empty()
 * scr_is_line_game_command()
 * scr_is_line_debug_command()
 * scr_is_line_command()
 * scr_is_line_empty_debug_command()
 *
 * Line classifiers, return TRUE if line has the given type.
 */
static sc_bool scr_is_line_type(const sc_char *line, sc_char type) {
	return line[0] == type;
}

static sc_bool scr_is_line_comment_or_empty(const sc_char *line) {
	return scr_is_line_type(line, SCRIPT_COMMENT)
	       || strspn(line, "\t\n\v\f\r ") == strlen(line);
}

static sc_bool scr_is_line_game_command(const sc_char *line) {
	return scr_is_line_type(line, GAME_COMMAND);
}

static sc_bool scr_is_line_debug_command(const sc_char *line) {
	return scr_is_line_type(line, DEBUG_COMMAND);
}

static sc_bool scr_is_line_command(const sc_char *line) {
	return scr_is_line_game_command(line) || scr_is_line_debug_command(line);
}

static sc_bool scr_is_line_empty_debug_command(const sc_char *line) {
	return scr_is_line_type(line, DEBUG_COMMAND) && line[1] == NUL;
}


/* Script location, a pair holding the file location and the line number. */
struct sx_scr_location_t {
	size_t position;
	sc_int line_number;
};
typedef sx_scr_location_t *sx_scr_locationref_t;

/*
 * scr_save_location()
 * scr_restore_location()
 *
 * Save and restore the script location in the given structure.
 */
static void scr_save_location(sx_script script, sx_scr_locationref_t location) {
	location->position = script->pos();
	location->line_number = scr_line_number;
}

static void scr_restore_location(sx_script script, sx_scr_locationref_t location) {
	script->seek(location->position);
	scr_line_number = location->line_number;
}


/*
 * scr_get_next_line()
 *
 * Helper for scr_get_next_section().  Returns the next non-comment, non-empty
 * line from the script.  Returns NULL if no more lines, or on file error.  The
 * return string is allocated, and it's the caller's responsibility to free it.
 */
static sc_char *scr_get_next_line(sx_script script) {
	sc_char *buffer, *line = NULL;

	/* Allocate a buffer for line reads. */
	buffer = (sc_char *)sx_malloc(LINE_BUFFER_SIZE);

	/* Read until a significant line is found, or end of file or error. */
	while (adrift_fgets(buffer, LINE_BUFFER_SIZE, script)) {
		scr_line_number++;
		if (!scr_is_line_comment_or_empty(buffer)) {
			line = buffer;
			break;
		}
	}

	/* If no significant line read, free the read buffer. */
	if (!line)
		sx_free(buffer);

	return line;
}


/*
 * scr_concatenate()
 *
 * Helper for scr_get_next_section().  Builds a string formed by concatenating
 * the second argument to the first.  If the first is NULL, acts as strdup()
 * instead.
 */
static sc_char *scr_concatenate(sc_char *string, const sc_char *buffer) {
	/* If string is not null, concatenate buffer, otherwise duplicate. */
	if (string) {
		string = (sc_char *)sx_realloc(string,
		                               strlen(string) + 1 + strlen(buffer) + 1);
		strcat(string, " ");
		strcat(string, buffer);
	} else {
		string = (sc_char *)sx_malloc(strlen(buffer) + 1);
		strcpy(string, buffer);
	}

	return string;
}


/*
 * scr_get_next_section()
 *
 * Retrieve the next command and any expectation from the script file.
 * Returns TRUE if a line is returned, FALSE at end-of-file.  Expectation may
 * be NULL if this paragraph doesn't have one; command may not be (if TRUE is
 * returned).  Command and expectation are allocated, and the caller needs to
 * free them.
 */
static sc_bool scr_get_next_section(sx_script script, sc_char **command, sc_char **expectation) {
	sc_char *line, *first_line, *other_lines;
	sx_scr_location_t location;

	/* Clear initial line accumulation. */
	first_line = other_lines = NULL;

	/* Read the next significant line from the script. */
	scr_save_location(script, &location);
	line = scr_get_next_line(script);
	while (line) {
		/* If already a first line, this is other lines or section end. */
		if (first_line) {
			/*
			 * If we found the start of the next section, reset the script
			 * location that saved on the line read, and we're done.
			 */
			if (scr_is_line_command(line)) {
				scr_restore_location(script, &location);
				sx_free(line);
				break;
			} else
				other_lines = scr_concatenate(other_lines, line);
		} else
			first_line = scr_concatenate(first_line, line);

		sx_free(line);

		/* Read the next significant line from the script. */
		scr_save_location(script, &location);
		line = scr_get_next_line(script);
	}

	/* Clean up and return nothing on file error. */
	if (script->err()) {
		scr_test_failed("Script error: Failed reading script input file", "");
		sx_free(first_line);
		sx_free(other_lines);
		return FALSE;
	}

	/* Return the command and the matching expectation string, if any. */
	if (first_line) {
		*command = sx_normalize_string(first_line);
		*expectation = other_lines ? sx_normalize_string(other_lines) : NULL;
		return TRUE;
	}

	/* End of file, no command section read. */
	return FALSE;
}


/*
 * scr_expect()
 * scr_verify_expectation()
 *
 * Set an expectation, and compare the expectation, if any, with the
 * accumulated game output, using glob matching.  scr_verify_expectation()
 * increments the error count if the expectation isn't met, and reports the
 * error if required.  It then frees both the expectation and accumulated
 * input.
 */
static void scr_expect(sc_char *expectation) {
	/*
	 * Save the expectation, and set up collection of game output if needed.
	 * And if not needed, ensure expectation and game output are cleared.
	 */
	if (expectation) {
		scr_expectation = (sc_char *)sx_malloc(strlen(expectation) + 1);
		strcpy(scr_expectation, expectation);
		scr_game_output = (sc_char *)sx_malloc(1);
		strcpy(scr_game_output, "");
	} else {
		sx_free(scr_expectation);
		scr_expectation = NULL;
		sx_free(scr_game_output);
		scr_game_output = NULL;
	}
}

static void scr_verify_expectation(void) {
	/* Compare expected with actual, and handle any error detected. */
	if (scr_expectation && scr_game_output) {
		scr_game_output = sx_normalize_string(scr_game_output);
		if (!glob_match(scr_expectation, scr_game_output)) {
			scr_test_failed("Expectation error:", "");
			scr_test_message("  Expected: \"%s\"", scr_expectation);
			scr_test_message("  Received: \"%s\"", scr_game_output);
		}
	}

	/* Dispose of the expectation and accumulated game output. */
	sx_free(scr_expectation);
	scr_expectation = NULL;
	sx_free(scr_game_output);
	scr_game_output = NULL;
}


/*
 * scr_execute_debugger_command()
 *
 * Convenience interface for immediate execution of debugger commands.  This
 * function directly calls the debugger interface, and because it's immediate,
 * can also verify the expectation before returning to the caller.
 *
 * Also, it turns on the game debugger, and it's the caller's responsibility
 * to turn it off when it's no longer needed.
 */
static void scr_execute_debugger_command(const sc_char *command, sc_char *expectation) {
	sc_bool status;

	/* Set up the expectation. */
	scr_expect(expectation);

	/*
	 * Execute the command via the debugger interface.  The "+1" on command
	 * skips the leading '~' read in from the game script.
	 */
	sc_set_game_debugger_enabled(scr_game, TRUE);
	status = sc_run_game_debugger_command(scr_game, command + 1);

	if (!status) {
		scr_test_failed("Script error:"
		                " Debug command \"%s\" is not valid", command);
	}

	/* Check expectations immediately. */
	scr_verify_expectation();
}


/*
 * scr_read_line_callback()
 *
 * Check any expectations set for the last line.  Consult the script for the
 * next line to feed to the game, and any expectation for the game output
 * for that line.  If there is an expectation, save it and set scr_game_output
 * to "" so that accumulation begins.  Then pass the next line of data back
 * to the game.
 */
static sc_bool scr_read_line_callback(sc_char *buffer, sc_int length) {
	sc_char *command, *expectation;
	assert(buffer && length > 0);

	/* Check pending expectation, and clear settings for the next line. */
	scr_verify_expectation();

	/* Get the next line-expectation pair from the script stream. */
	if (scr_get_next_section(scr_script, &command, &expectation)) {
		if (scr_is_line_debug_command(command)) {
			/* The debugger persists where debug commands are adjacent. */
			scr_execute_debugger_command(command, expectation);
			sx_free(command);
			sx_free(expectation);

			/*
			 * Returning FALSE here causes the game to re-prompt.  We could
			 * loop (or tail recurse) ourselves, but returning is simpler.
			 */
			return FALSE;
		} else
			sc_set_game_debugger_enabled(scr_game, FALSE);

		if (scr_is_line_game_command(command)) {
			/* Set up the expectation. */
			scr_expect(expectation);

			/* Copy out the line to the return buffer, and free the line. */
			strncpy(buffer, command + 1, length);
			buffer[length - 1] = NUL;
			sx_free(command);
			sx_free(expectation);
			return TRUE;
		}

		/* Neither a '~' nor a '>' command. */
		scr_test_failed("Script error:"
		                " Command \"%s\" is not valid, ignored", command);
		sx_free(command);
		sx_free(expectation);
		return FALSE;
	}

	/* Ensure the game debugger is off after this section. */
	sc_set_game_debugger_enabled(scr_game, FALSE);

	/*
	 * We reached the end of the script without finding a "quit" command.
	 * Supply one here, then.  In the unlikely even that this does not quit
	 * the game, we'll iterate on this.
	 */
	assert(length > 4);
	strcpy(buffer, "quit");
	return TRUE;
}


/*
 * scr_print_string_callback()
 *
 * Handler function for game output.  Accumulates strings received from the
 * game into scr_game_output, unless no expectation is set, in which case
 * the current game output will be NULL, and we can simply save the effort.
 */
static void scr_print_string_callback(const sc_char *string) {
	assert(string);

	if (scr_game_output) {
		scr_game_output = (sc_char *)sx_realloc(scr_game_output,
		                                        strlen(scr_game_output)
		                                        + strlen(string) + 1);
		strcat(scr_game_output, string);
	}
}


/*
 * scr_start_script()
 *
 * Set up game monitoring so that each request for a line from the game
 * enters this module.  For each request, we grab the next "send" and
 * "expect" pair from the script, satisfy the request with the send data,
 * and match against the expectations on next request or on finalization.
 */
void scr_start_script(sc_game game, sx_script script) {
	sc_char *command, *expectation;
	sx_scr_location_t location;
	assert(game && script);

	/* Save the game and stream, and clear the line number and errors count. */
	assert(!scr_game && !scr_script);
	scr_game = game;
	scr_script = script;
	scr_line_number = 0;
	scr_errors = 0;

	/* Set up our callback functions to catch game i/o. */
	stub_attach_handlers(scr_read_line_callback, scr_print_string_callback,
	                     file_open_file_callback, file_read_file_callback,
	                     file_write_file_callback, file_close_file_callback);

	/*
	 * Handle any initial debugging commands, terminating on either a non-
	 * debugging one or an expectation for the game intro.
	 */
	scr_script->seek(0);
	scr_save_location(scr_script, &location);
	while (scr_get_next_section(scr_script, &command, &expectation)) {
		if (scr_is_line_debug_command(command)) {
			if (scr_is_line_empty_debug_command(command)) {
				/* It's an intro expectation - set and break loop. */
				scr_expect(expectation);
				sx_free(command);
				sx_free(expectation);
				break;
			} else {
				/* It's a full debug command - execute it as one. */
				scr_execute_debugger_command(command, expectation);
				sx_free(command);
				sx_free(expectation);
			}
		} else {
			/*
			 * It's an ordinary section - rewind so that it's the first one
			 * handled in the callback, and break loop.
			 */
			scr_restore_location(scr_script, &location);
			sx_free(command);
			sx_free(expectation);
			break;
		}

		/* Note script position before reading the next section. */
		scr_save_location(scr_script, &location);
	}

	/* Ensure the game debugger is off after this section. */
	sc_set_game_debugger_enabled(scr_game, FALSE);
}


/*
 * scr_finalize_script()
 *
 * Match any final received string against a possible expectation, and then
 * clear local records of the game, stream, and error count.  Returns the
 * count of errors detected during the script.
 */
sc_int scr_finalize_script(void) {
	sc_char *command, *expectation;
	sc_int errors;

	/* Check pending expectation, and clear settings. */
	scr_verify_expectation();

	/* Drain the remainder of the script, ignoring non-debugging commands. */
	while (scr_get_next_section(scr_script, &command, &expectation)) {
		if (scr_is_line_debug_command(command)) {
			scr_execute_debugger_command(command, expectation);
			sx_free(command);
			sx_free(expectation);
		} else {
			/* Complain about script entries ignored because the game ended. */
			scr_test_failed("Script error:"
			                " Game completed, command \"%s\" ignored", command);
			sx_free(command);
			sx_free(expectation);
		}
	}

	/* Ensure the game debugger is off after this section. */
	sc_set_game_debugger_enabled(scr_game, FALSE);

	/*
	 * Remove our callback functions from the stubs, and "close" any retained
	 * stream data from game save/load tests.
	 */
	stub_detach_handlers();
	file_cleanup();

	/* Clear local records of game stream, line number, and errors count. */
	errors = scr_errors;
	scr_game = NULL;
	scr_script = NULL;
	scr_line_number = 0;
	scr_errors = 0;

	return errors;
}

} // End of namespace Adrift
} // End of namespace Glk
