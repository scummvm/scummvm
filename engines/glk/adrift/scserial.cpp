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
#include "glk/adrift/scprotos.h"
#include "glk/adrift/scgamest.h"
#include "common/textconsole.h"

namespace Glk {
namespace Adrift {

/* Assorted definitions and constants. */
static const sc_char NEWLINE = '\n';
static const sc_char CARRIAGE_RETURN = '\r';
static const sc_char NUL = '\0';

enum { BUFFER_SIZE = 4096 };

/* Output buffer. */
static sc_byte *ser_buffer = NULL;
static sc_int ser_buffer_length = 0;

/* Callback and opaque pointer for use by output functions. */
static sc_write_callbackref_t ser_callback = NULL;
static void *ser_opaque = NULL;


/*
 * ser_flush()
 * ser_buffer_character()
 *
 * Flush pending buffer contents; add a character to the buffer.
 */
static void ser_flush(sc_bool is_final) {
	error("TODO");
#ifdef TODO
	static sc_bool initialized = FALSE;
	static sc_byte *out_buffer = NULL;
	static sc_int out_buffer_size = 0;
	static z_stream stream;

	sc_int status;

	/* If this is an initial call, initialize deflation. */
	if (!initialized) {
		/* Allocate an initial output buffer. */
		out_buffer_size = BUFFER_SIZE;
		out_buffer = sc_malloc(out_buffer_size);

		/* Initialize Zlib deflation functions. */
		stream.next_out = out_buffer;
		stream.avail_out = out_buffer_size;
		stream.next_in = ser_buffer;
		stream.avail_in = 0;

		stream.zalloc = Z_NULL;
		stream.zfree = Z_NULL;
		stream.opaque = Z_NULL;

		status = deflateInit(&stream, Z_DEFAULT_COMPRESSION);
		if (status != Z_OK) {
			sc_error("ser_flush: deflateInit: error %ld\n", status);
			ser_buffer_length = 0;

			sc_free(out_buffer);
			out_buffer = NULL;
			out_buffer_size = 0;
			return;
		}

		initialized = TRUE;
	}

	/* Deflate data from the current output buffer. */
	stream.next_in = ser_buffer;
	stream.avail_in = ser_buffer_length;

	/* Loop while deflate output is pending and buffer not emptied. */
	while (TRUE) {
		sc_int in_bytes, out_bytes;

		/* Compress stream data, with finish if this is the final flush. */
		if (is_final)
			status = deflate(&stream, Z_FINISH);
		else
			status = deflate(&stream, Z_NO_FLUSH);
		if (status != Z_STREAM_END && status != Z_OK) {
			sc_error("ser_flush: deflate: error %ld\n", status);
			ser_buffer_length = 0;

			sc_free(out_buffer);
			out_buffer = NULL;
			out_buffer_size = 0;
			initialized = FALSE;
			return;
		}

		/* Calculate bytes used, and output. */
		in_bytes = ser_buffer_length - stream.avail_in;
		out_bytes = out_buffer_size - stream.avail_out;

		/* See if compressed data is available. */
		if (out_bytes > 0) {
			/* Write it to save file output through the callback. */
			ser_callback(ser_opaque, out_buffer, out_bytes);

			/* Reset deflation stream for available space. */
			stream.next_out = out_buffer;
			stream.avail_out = out_buffer_size;
		}

		/* Remove consumed data from the input buffer. */
		if (in_bytes > 0) {
			/* Move any unused data, and reduce length. */
			memmove(ser_buffer,
			        ser_buffer + in_bytes, ser_buffer_length - in_bytes);
			ser_buffer_length -= in_bytes;

			/* Reset deflation stream for consumed data. */
			stream.next_in = ser_buffer;
			stream.avail_in = ser_buffer_length;
		}

		/* If final flush, wait until deflate indicates finished. */
		if (is_final && status == Z_OK)
			continue;

		/* If data was consumed or produced, break. */
		if (out_bytes > 0 || in_bytes > 0)
			break;
	}

	/* If this was a final call, clean up. */
	if (is_final) {
		/* Compression completed. */
		status = deflateEnd(&stream);
		if (status != Z_OK)
			sc_error("ser_flush: warning: deflateEnd: error %ld\n", status);

		if (ser_buffer_length != 0) {
			sc_error("ser_flush: warning: deflate missed data\n");
			ser_buffer_length = 0;
		}

		/* Free the allocated compression buffer. */
		sc_free(ser_buffer);
		ser_buffer = NULL;

		/*
		 * Free output buffer, and reset flag for reinitialization on the next
		 * call.
		 */
		sc_free(out_buffer);
		out_buffer = NULL;
		out_buffer_size = 0;
		initialized = FALSE;
	}
#endif
}

static void ser_buffer_character(sc_char character) {
	/* Allocate the buffer if not yet done. */
	if (!ser_buffer) {
		assert(ser_buffer_length == 0);
		ser_buffer = (sc_byte *)sc_malloc(BUFFER_SIZE);
	}

	/* Add to the buffer, with intermediate flush if filled. */
	ser_buffer[ser_buffer_length++] = character;
	if (ser_buffer_length == BUFFER_SIZE)
		ser_flush(FALSE);
}


/*
 * ser_buffer_buffer()
 * ser_buffer_string()
 * ser_buffer_int()
 * ser_buffer_int_special()
 * ser_buffer_uint()
 * ser_buffer_boolean()
 *
 * Buffer a buffer, a string, an unsigned and signed integer, and a boolean.
 */
static void ser_buffer_buffer(const sc_char *buffer, sc_int length) {
	sc_int index_;

	/* Add each character to the buffer. */
	for (index_ = 0; index_ < length; index_++)
		ser_buffer_character(buffer[index_]);
}

static void ser_buffer_string(const sc_char *string) {
	/* Buffer string, followed by DOS style end-of-line. */
	ser_buffer_buffer(string, strlen(string));
	ser_buffer_character(CARRIAGE_RETURN);
	ser_buffer_character(NEWLINE);
}

static void ser_buffer_int(sc_int value) {
	sc_char buffer[32];

	/* Convert to a string and buffer that. */
	sprintf(buffer, "%ld", value);
	ser_buffer_string(buffer);
}

static void ser_buffer_int_special(sc_int value) {
	sc_char buffer[32];

	/* Weirdo formatting for compatibility. */
	sprintf(buffer, "% ld ", value);
	ser_buffer_string(buffer);
}

static void ser_buffer_uint(sc_uint value) {
	sc_char buffer[32];

	/* Convert to a string and buffer that. */
	sprintf(buffer, "%lu", value);
	ser_buffer_string(buffer);
}

static void ser_buffer_boolean(sc_bool boolean) {
	/* Write a 1 for TRUE, 0 for FALSE. */
	ser_buffer_string(boolean ? "1" : "0");
}


/*
 * ser_save_game()
 *
 * Serialize a game and save its state using the given callback and opaque.
 */
void ser_save_game(sc_gameref_t game, sc_write_callbackref_t callback, void *opaque) {
	const sc_var_setref_t vars = gs_get_vars(game);
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[3];
	sc_int index_, var_count;
	assert(callback);

	/* Store the callback and opaque references, for writer functions. */
	ser_callback = callback;
	ser_opaque = opaque;

	/* Write the game name. */
	vt_key[0].string = "Globals";
	vt_key[1].string = "GameName";
	ser_buffer_string(prop_get_string(bundle, "S<-ss", vt_key));

	/* Write the counts of rooms, objects, etc. */
	ser_buffer_int(gs_room_count(game));
	ser_buffer_int(gs_object_count(game));
	ser_buffer_int(gs_task_count(game));
	ser_buffer_int(gs_event_count(game));
	ser_buffer_int(gs_npc_count(game));

	/* Write the score and player information. */
	ser_buffer_int(game->score);
	ser_buffer_int(gs_playerroom(game) + 1);
	ser_buffer_int(gs_playerparent(game));
	ser_buffer_int(gs_playerposition(game));

	/* Write player gender. */
	vt_key[0].string = "Globals";
	vt_key[1].string = "PlayerGender";
	ser_buffer_int(prop_get_integer(bundle, "I<-ss", vt_key));

	/*
	 * Write encumbrance details. The player limits are constant for a given
	 * game, and can be extracted from properties.  The current sizes and
	 * weights can also be recalculated from held objects, so we don't maintain
	 * them in the game.  We can write constants here, then, and ignore
	 * the values on restoring.  Note however that if the Adrift Runner is
	 * relying on these values, this may give it problems with one of our saved
	 * games.
	 */
	ser_buffer_int(90);
	ser_buffer_int(0);
	ser_buffer_int(90);
	ser_buffer_int(0);

	/* Save rooms information. */
	for (index_ = 0; index_ < gs_room_count(game); index_++)
		ser_buffer_boolean(gs_room_seen(game, index_));

	/* Save objects information. */
	for (index_ = 0; index_ < gs_object_count(game); index_++) {
		ser_buffer_int(gs_object_position(game, index_));
		ser_buffer_boolean(gs_object_seen(game, index_));
		ser_buffer_int(gs_object_parent(game, index_));
		if (gs_object_openness(game, index_) != 0)
			ser_buffer_int(gs_object_openness(game, index_));

		if (gs_object_state(game, index_) != 0)
			ser_buffer_int(gs_object_state(game, index_));

		ser_buffer_boolean(gs_object_unmoved(game, index_));
	}

	/* Save tasks information. */
	for (index_ = 0; index_ < gs_task_count(game); index_++) {
		ser_buffer_boolean(gs_task_done(game, index_));
		ser_buffer_boolean(gs_task_scored(game, index_));
	}

	/* Save events information. */
	for (index_ = 0; index_ < gs_event_count(game); index_++) {
		sc_int startertype, task;

		/* Get starter task, if any. */
		vt_key[0].string = "Events";
		vt_key[1].integer = index_;
		vt_key[2].string = "StarterType";
		startertype = prop_get_integer(bundle, "I<-sis", vt_key);
		if (startertype == 3) {
			vt_key[2].string = "TaskNum";
			task = prop_get_integer(bundle, "I<-sis", vt_key);
		} else
			task = 0;

		/* Save event details. */
		ser_buffer_int(gs_event_time(game, index_));
		ser_buffer_int(task);
		ser_buffer_int(gs_event_state(game, index_) - 1);
		if (task > 0)
			ser_buffer_boolean(gs_task_done(game, task - 1));
		else
			ser_buffer_boolean(FALSE);
	}

	/* Save NPCs information. */
	for (index_ = 0; index_ < gs_npc_count(game); index_++) {
		sc_int walk;

		ser_buffer_int(gs_npc_location(game, index_));
		ser_buffer_boolean(gs_npc_seen(game, index_));
		for (walk = 0; walk < gs_npc_walkstep_count(game, index_); walk++)
			ser_buffer_int_special(gs_npc_walkstep(game, index_, walk));
	}

	/* Save each variable. */
	vt_key[0].string = "Variables";
	var_count = prop_get_child_count(bundle, "I<-s", vt_key);

	for (index_ = 0; index_ < var_count; index_++) {
		const sc_char *name;
		sc_int var_type;

		vt_key[1].integer = index_;

		vt_key[2].string = "Name";
		name = prop_get_string(bundle, "S<-sis", vt_key);
		vt_key[2].string = "Type";
		var_type = prop_get_integer(bundle, "I<-sis", vt_key);

		switch (var_type) {
		case TAFVAR_NUMERIC:
			ser_buffer_int(var_get_integer(vars, name));
			break;

		case TAFVAR_STRING:
			ser_buffer_string(var_get_string(vars, name));
			break;

		default:
			sc_fatal("ser_save_game: unknown variable type, %ld\n", var_type);
		}
	}

	/* Save timing information. */
	ser_buffer_uint(var_get_elapsed_seconds(vars));

	/* Save turns count. */
	ser_buffer_uint((sc_uint) game->turns);

	/*
	 * Flush the last buffer contents, and drop the callback and opaque
	 * references.
	 */
	ser_flush(TRUE);
	ser_callback = NULL;
	ser_opaque = NULL;
}


/*
 * ser_save_game_prompted()
 *
 * Serialize a game and save its state, requesting a save stream from
 * the user.
 */
sc_bool ser_save_game_prompted(sc_gameref_t game) {
	void *opaque;

	/*
	 * Open an output stream, and if successful, save a game using the opaque
	 * value returned.
	 */
	opaque = if_open_saved_game(TRUE);
	if (opaque) {
		ser_save_game(game, if_write_saved_game, opaque);
		if_close_saved_game(opaque);
		return TRUE;
	}

	return FALSE;
}


/* TAS input file line counter. */
static sc_tafref_t ser_tas = NULL;
static sc_int ser_tasline = 0;

/* Restore error jump buffer. */
static jmp_buf ser_tas_error;

/*
 * ser_get_string()
 * ser_get_int()
 * ser_get_uint()
 * ser_get_boolean()
 *
 * Wrapper round obtaining the next TAS file line, with variants to convert
 * the line content into an appropriate type.
 */
static const sc_char *ser_get_string(void) {
	const sc_char *string;

	/* Get the next line, and complain if absent. */
	string = taf_next_line(ser_tas);
	if (!string) {
		sc_error("ser_get_string: out of TAS data at line %ld\n", ser_tasline);
		longjmp(ser_tas_error, 1);
	}

	ser_tasline++;
	return string;
}

static sc_int ser_get_int(void) {
	const sc_char *string;
	sc_int value;

	/* Get line, and scan for a single integer; return it. */
	string = ser_get_string();
	if (sscanf(string, "%ld", &value) != 1) {
		sc_error("ser_get_int:"
		         " invalid integer at line %ld\n", ser_tasline - 1);
		longjmp(ser_tas_error, 1);
	}

	return value;
}

static sc_uint ser_get_uint(void) {
	const sc_char *string;
	sc_uint value;

	/* Get line, and scan for a single integer; return it. */
	string = ser_get_string();
	if (sscanf(string, "%lu", &value) != 1) {
		sc_error("ser_get_uint:"
		         " invalid integer at line %ld\n", ser_tasline - 1);
		longjmp(ser_tas_error, 1);
	}

	return value;
}

static sc_bool ser_get_boolean(void) {
	const sc_char *string;
	sc_uint value;

	/*
	 * Get line, and scan for a single integer; check it's a valid-looking flag,
	 * and return it.
	 */
	string = ser_get_string();
	if (sscanf(string, "%lu", &value) != 1) {
		sc_error("ser_get_boolean:"
		         " invalid boolean at line %ld\n", ser_tasline - 1);
		longjmp(ser_tas_error, 1);
	}
	if (value != 0 && value != 1) {
		sc_error("ser_get_boolean:"
		         " warning: suspect boolean at line %ld\n", ser_tasline - 1);
	}

	return value != 0;
}


/*
 * ser_load_game()
 *
 * Load a serialized game into the given game by repeated calls to the
 * callback() function.
 */
sc_bool ser_load_game(sc_gameref_t game, sc_read_callbackref_t callback, void *opaque) {
	static sc_var_setref_t new_vars;  /* For setjmp safety */
	static sc_gameref_t new_game;     /* For setjmp safety */

	const sc_filterref_t filter = gs_get_filter(game);
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[3];
	sc_int index_, var_count;
	const sc_char *gamename;

	/* Create a TAF (TAS) reference from callbacks, for reader functions. */
	ser_tas = taf_create_tas(callback, opaque);
	if (!ser_tas)
		return FALSE;

	/* Reset line counter for error messages. */
	ser_tasline = 1;

	new_game = NULL;
	new_vars = NULL;

	/* Set up error handling jump buffer, and handle errors. */
	if (setjmp(ser_tas_error) != 0) {
		/* Destroy any temporary game and variables. */
		if (new_game)
			gs_destroy(new_game);
		if (new_vars)
			var_destroy(new_vars);

		/* Destroy the TAF (TAS) file and return fail status. */
		taf_destroy(ser_tas);
		ser_tas = NULL;
		return FALSE;
	}

	/*
	 * Read the game name, and compare with the one in the game.  Fail if
	 * they don't match exactly.  A tighter check than this would perhaps be
	 * preferable, say, something based on the TAF file header, but this isn't
	 * in the save file format.
	 */
	vt_key[0].string = "Globals";
	vt_key[1].string = "GameName";
	gamename = prop_get_string(bundle, "S<-ss", vt_key);
	if (strcmp(ser_get_string(), gamename) != 0)
		longjmp(ser_tas_error, 1);

	/* Read and verify the counts in the saved game. */
	if (ser_get_int() != gs_room_count(game)
	        || ser_get_int() != gs_object_count(game)
	        || ser_get_int() != gs_task_count(game)
	        || ser_get_int() != gs_event_count(game)
	        || ser_get_int() != gs_npc_count(game))
		longjmp(ser_tas_error, 1);

	/* Create a variables set and game to restore into. */
	new_vars = var_create(bundle);
	new_game = gs_create(new_vars, bundle, filter);
	var_register_game(new_vars, new_game);

	/* All set to load TAF (TAS) data into the new game. */

	/* Restore the score and player information. */
	new_game->score = ser_get_int();
	gs_set_playerroom(new_game, ser_get_int() - 1);
	gs_set_playerparent(new_game, ser_get_int());
	gs_set_playerposition(new_game, ser_get_int());

	/* Skip player gender. */
	(void) ser_get_int();

	/* Skip encumbrance details, not currently maintained by the game. */
	(void) ser_get_int();
	(void) ser_get_int();
	(void) ser_get_int();
	(void) ser_get_int();

	/* Restore rooms information. */
	for (index_ = 0; index_ < gs_room_count(new_game); index_++)
		gs_set_room_seen(new_game, index_, ser_get_boolean());

	/* Restore objects information. */
	for (index_ = 0; index_ < gs_object_count(new_game); index_++) {
		sc_int openable, currentstate;

		/* Bypass mutators for position and parent.  Fix later? */
		new_game->objects[index_].position = ser_get_int();
		gs_set_object_seen(new_game, index_, ser_get_boolean());
		new_game->objects[index_].parent = ser_get_int();

		vt_key[0].string = "Objects";
		vt_key[1].integer = index_;
		vt_key[2].string = "Openable";
		openable = prop_get_integer(bundle, "I<-sis", vt_key);
		gs_set_object_openness(new_game, index_,
		                       openable != 0 ? ser_get_int() : 0);

		vt_key[2].string = "CurrentState";
		currentstate = prop_get_integer(bundle, "I<-sis", vt_key);
		gs_set_object_state(new_game, index_,
		                    currentstate != 0 ? ser_get_int() : 0);

		gs_set_object_unmoved(new_game, index_, ser_get_boolean());
	}

	/* Restore tasks information. */
	for (index_ = 0; index_ < gs_task_count(new_game); index_++) {
		gs_set_task_done(new_game, index_, ser_get_boolean());
		gs_set_task_scored(new_game, index_, ser_get_boolean());
	}

	/* Restore events information. */
	for (index_ = 0; index_ < gs_event_count(new_game); index_++) {
		sc_int startertype, task;

		/* Restore first event details. */
		gs_set_event_time(new_game, index_, ser_get_int());
		task = ser_get_int();
		gs_set_event_state(new_game, index_, ser_get_int() + 1);

		/* Verify and restore the starter task, if any. */
		if (task > 0) {
			vt_key[0].string = "Events";
			vt_key[1].integer = index_;
			vt_key[2].string = "StarterType";
			startertype = prop_get_integer(bundle, "I<-sis", vt_key);
			if (startertype != 3)
				longjmp(ser_tas_error, 1);

			/* Restore task state. */
			gs_set_task_done(new_game, task - 1, ser_get_boolean());
		} else
			(void) ser_get_boolean();
	}

	/* Restore NPCs information. */
	for (index_ = 0; index_ < gs_npc_count(new_game); index_++) {
		sc_int walk;

		gs_set_npc_location(new_game, index_, ser_get_int());
		gs_set_npc_seen(new_game, index_, ser_get_boolean());
		for (walk = 0; walk < gs_npc_walkstep_count(new_game, index_); walk++)
			gs_set_npc_walkstep(new_game, index_, walk, ser_get_int());
	}

	/* Restore each variable. */
	vt_key[0].string = "Variables";
	var_count = prop_get_child_count(bundle, "I<-s", vt_key);

	for (index_ = 0; index_ < var_count; index_++) {
		const sc_char *name;
		sc_int var_type;

		vt_key[1].integer = index_;

		vt_key[2].string = "Name";
		name = prop_get_string(bundle, "S<-sis", vt_key);
		vt_key[2].string = "Type";
		var_type = prop_get_integer(bundle, "I<-sis", vt_key);

		switch (var_type) {
		case TAFVAR_NUMERIC:
			var_put_integer(new_vars, name, ser_get_int());
			break;

		case TAFVAR_STRING:
			var_put_string(new_vars, name, ser_get_string());
			break;

		default:
			sc_fatal("ser_load_game: unknown variable type, %ld\n", var_type);
		}
	}

	/* Restore timing information. */
	var_set_elapsed_seconds(new_vars, ser_get_uint());

	/* Restore turns count. */
	new_game->turns = (sc_int) ser_get_uint();

	/*
	 * Resources tweak -- set requested to match those in the current game
	 * so that they remain unchanged by the gs_copy() of new_game onto
	 * game.  This way, both the requested and the active resources in the
	 * game are unchanged by restore.
	 */
	new_game->requested_sound = game->requested_sound;
	new_game->requested_graphic = game->requested_graphic;

	/*
	 * Quitter tweak -- set the quit jump buffer in the new game to be the
	 * same as the current one, so that it remains unchanged by gs_copy().  The
	 * one in the new game is still the unset one from gs_create().
	 */
	memcpy(&new_game->quitter, &game->quitter, sizeof(game->quitter));

	/*
	 * If we got this far, we successfully restored the game from the file.
	 * As our final act, copy the new game onto the old one.
	 */
	new_game->temporary = game->temporary;
	new_game->undo = game->undo;
	gs_copy(game, new_game);

	/* Done with the temporary game and variables. */
	gs_destroy(new_game);
	var_destroy(new_vars);

	/* Done with TAF (TAS) file; destroy it and return successfully. */
	taf_destroy(ser_tas);
	ser_tas = NULL;
	return TRUE;
}


/*
 * ser_load_game_prompted()
 *
 * Load a serialized game into the given game, requesting a restore
 * stream from the user.
 */
sc_bool ser_load_game_prompted(sc_gameref_t game) {
	void *opaque;

	/*
	 * Open an input stream, and if successful, try to load a game using
	 * the opaque value returned and the saved game callback.
	 */
	opaque = if_open_saved_game(FALSE);
	if (opaque) {
		sc_bool status;

		status = ser_load_game(game, if_read_saved_game, opaque);
		if_close_saved_game(opaque);
		return status;
	}

	return FALSE;
}

} // End of namespace Adrift
} // End of namespace Glk
