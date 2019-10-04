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

#include "glk/jacl/jacl.h"
#include "glk/jacl/language.h"
#include "glk/jacl/types.h"
#include "glk/jacl/prototypes.h"

namespace Glk {
namespace JACL {

extern struct object_type       *object[];
extern struct integer_type      *integer_table;
extern struct integer_type      *integer[];
extern struct function_type     *function_table;
extern struct string_type       *string_table;

extern schanid_t                sound_channel[];

extern char                     temp_buffer[];

extern int                      objects;
extern int                      integers;
extern int                      functions;
extern int                      strings;
extern int                      player;

extern int                      it;
extern int                      them[];
extern int                      her;
extern int                      him;
extern int                      parent;

extern int                      noun[];

int save_game(frefid_t saveref) {
	struct integer_type *current_integer = integer_table;
	struct function_type *current_function = function_table;
	struct string_type *current_string = string_table;

	int             index, counter;
	strid_t         bookmark = NULL;

	bookmark = g_vm->glk_stream_open_file(saveref, filemode_Write, 0);

	if (bookmark == NULL) {
		return (FALSE);
	}

	/* WE'RE DONE WITH THE FILE REFERENCE NOW THAT THE STREAM
	 * HAS BEEN SUCCESSFULLY OPENED */
	g_vm->glk_fileref_destroy(saveref);

	/* THIS IS WRITTEN TO HELP VALIDATE THE SAVED GAME
	 * BEFORE CONTINUING TO LOAD IT */
	write_integer(bookmark, objects);
	write_integer(bookmark, integers);
	write_integer(bookmark, functions);
	write_integer(bookmark, strings);

	while (current_integer != NULL) {
		write_integer(bookmark, current_integer->value);
		current_integer = current_integer->next_integer;
	}

	while (current_function != NULL) {
		write_integer(bookmark, current_function->call_count);
		current_function = current_function->next_function;
	}

	for (index = 1; index <= objects; index++) {
		if (object[index]->nosave)
			continue;

		for (counter = 0; counter < 16; counter++) {
			write_integer(bookmark, object[index]->integer[counter]);
		}

		write_long(bookmark, object[index]->attributes);
		write_long(bookmark, object[index]->user_attributes);
	}

	/* WRITE OUT ALL THE CURRENT VALUES OF THE STRING VARIABLES */
	while (current_string != NULL) {
		for (index = 0; index < 255; index++) {
			g_vm->glk_put_char_stream(bookmark, current_string->value[index]);
		}
		current_string = current_string->next_string;
	}

	write_integer(bookmark, player);
	write_integer(bookmark, noun[3]);

	/* SAVE THE CURRENT VOLUME OF EACH OF THE SOUND CHANNELS */
	for (index = 0; index < 8; index++) {
		sprintf(temp_buffer, "volume[%d]", index);
		write_integer(bookmark, cinteger_resolve(temp_buffer)->value);
	}

	/* SAVE THE CURRENT VALUE OF THE GLK TIMER */
	write_integer(bookmark, cinteger_resolve("timer")->value);

	/* CLOSE THE STREAM */
	g_vm->glk_stream_close(bookmark, NULL);

	TIME->value = FALSE;
	return (TRUE);
}

int restore_game(frefid_t saveref, int warn) {
	struct integer_type *current_integer = integer_table;
	struct function_type *current_function = function_table;
	struct string_type *current_string = string_table;

	int             index, counter;
	int             file_objects,
	                file_integers,
	                file_functions,
	                file_strings;
	strid_t         bookmark;

	bookmark = g_vm->glk_stream_open_file(saveref, filemode_Read, 0);

	if (!bookmark) {
		return (FALSE);
	}

	/* WE'RE DONE WITH THE FILE REFERENCE NOW THAT THE STREAM
	 * HAS BEEN SUCCESSFULLY OPENED */
	g_vm->glk_fileref_destroy(saveref);

	/* THIS IS WRITTEN TO HELP VALIDATE THE SAVED GAME
	 * BEFORE CONTINUING TO LOAD IT */
	file_objects = read_integer(bookmark);
	file_integers = read_integer(bookmark);
	file_functions = read_integer(bookmark);
	file_strings = read_integer(bookmark);

	if (file_objects != objects
	        || file_integers != integers
	        || file_functions != functions
	        || file_strings != strings) {
		if (warn == FALSE) {
			log_error(cstring_resolve("BAD_SAVED_GAME")->value, PLUS_STDOUT);
		}
		g_vm->glk_stream_close(bookmark, NULL);
		return (FALSE);
	}

	while (current_integer != NULL) {
		current_integer->value = read_integer(bookmark);
		current_integer = current_integer->next_integer;
	}

	while (current_function != NULL) {
		current_function->call_count = read_integer(bookmark);
		current_function = current_function->next_function;
	}

	for (index = 1; index <= objects; index++) {
		if (object[index]->nosave)
			continue;

		for (counter = 0; counter < 16; counter++) {
			object[index]->integer[counter] = read_integer(bookmark);
		}

		object[index]->attributes = read_integer(bookmark);
		object[index]->user_attributes = read_integer(bookmark);
	}

	while (current_string != NULL) {
		for (index = 0; index < 255; index++) {
			current_string->value[index] = g_vm->glk_get_char_stream(bookmark);
		}
		current_string = current_string->next_string;
	}

	player = read_integer(bookmark);
	noun[3] = read_integer(bookmark);

	/* RESTORE THE CURRENT VOLUME OF EACH OF THE SOUND CHANNELS */
	for (index = 0; index < 8; index++) {
		sprintf(temp_buffer, "volume[%d]", index);
		counter = read_integer(bookmark);
		cinteger_resolve(temp_buffer)->value = counter;

		if (SOUND_SUPPORTED->value) {
			/* SET THE GLK VOLUME */
			g_vm->glk_schannel_set_volume(sound_channel[index], (glui32) counter);
		}
	}

	/* RESTORE THE CURRENT VALUE OF THE GLK TIMER */
	counter = read_integer(bookmark);
	cinteger_resolve("timer")->value = counter;

	/* SET THE GLK TIMER */
	g_vm->glk_request_timer_events((glui32) counter);

	/* CLOSE THE STREAM */
	g_vm->glk_stream_close(bookmark, NULL);

	TIME->value = FALSE;
	return (TRUE);
}

void write_integer(strid_t stream, int x) {
	unsigned char c;

	c = (unsigned char)(x) & 0xFF;
	g_vm->glk_put_char_stream(stream, c);
	c = (unsigned char)(x >> 8) & 0xFF;
	g_vm->glk_put_char_stream(stream, c);
	c = (unsigned char)(x >> 16) & 0xFF;
	g_vm->glk_put_char_stream(stream, c);
	c = (unsigned char)(x >> 24) & 0xFF;
	g_vm->glk_put_char_stream(stream, c);
}

int read_integer(strid_t stream) {
	int a, b, c, d;
	a = (int) g_vm->glk_get_char_stream(stream);
	b = (int) g_vm->glk_get_char_stream(stream);
	c = (int) g_vm->glk_get_char_stream(stream);
	d = (int) g_vm->glk_get_char_stream(stream);
	return a | (b << 8) | (c << 16) | (d << 24);
}

void write_long(strid_t stream, long x) {
	unsigned char c;

	c = (unsigned char)(x) & 0xFF;
	g_vm->glk_put_char_stream(stream, c);
	c = (unsigned char)(x >> 8) & 0xFF;
	g_vm->glk_put_char_stream(stream, c);
	c = (unsigned char)(x >> 16) & 0xFF;
	g_vm->glk_put_char_stream(stream, c);
	c = (unsigned char)(x >> 24) & 0xFF;
	g_vm->glk_put_char_stream(stream, c);
}

long read_long(strid_t stream) {
	long a, b, c, d;
	a = (long) g_vm->glk_get_char_stream(stream);
	b = (long) g_vm->glk_get_char_stream(stream);
	c = (long) g_vm->glk_get_char_stream(stream);
	d = (long) g_vm->glk_get_char_stream(stream);
	return a | (b << 8) | (c << 16) | (d << 24);
}

} // End of namespace JACL
} // End of namespace Glk
