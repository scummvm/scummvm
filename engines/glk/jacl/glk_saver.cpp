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

bool save_game(strid_t save) {
	integer_type *current_integer = integer_table;
	function_type *current_function = function_table;
	string_type *current_string = string_table;
	int index, counter;

	// This is written to help validate the saved game when it's loaded
	write_integer(save, objects);
	write_integer(save, integers);
	write_integer(save, functions);
	write_integer(save, strings);

	while (current_integer != NULL) {
		write_integer(save, current_integer->value);
		current_integer = current_integer->next_integer;
	}

	while (current_function != NULL) {
		write_integer(save, current_function->call_count);
		current_function = current_function->next_function;
	}

	for (index = 1; index <= objects; index++) {
		if (object[index]->nosave)
			continue;

		for (counter = 0; counter < 16; counter++) {
			write_integer(save, object[index]->integer[counter]);
		}

		write_long(save, object[index]->attributes);
		write_long(save, object[index]->user_attributes);
	}

	// Write out all the current values of the string variables
	while (current_string != NULL) {
		for (index = 0; index < 255; index++) {
			g_vm->glk_put_char_stream(save, current_string->value[index]);
		}
		current_string = current_string->next_string;
	}

	write_integer(save, player);
	write_integer(save, noun[3]);

	// Save the current volume of each of the sound channels
	for (index = 0; index < 8; index++) {
		sprintf(temp_buffer, "volume[%d]", index);
		write_integer(save, cinteger_resolve(temp_buffer)->value);
	}

	// Save the current value of the GLK timer
	write_integer(save, cinteger_resolve("timer")->value);

	TIME->value = FALSE;
	return true;
}

bool restore_game(strid_t save, bool warn) {
	integer_type *current_integer = integer_table;
	function_type *current_function = function_table;
	string_type *current_string = string_table;

	int index, counter;
	int file_objects, file_integers, file_functions, file_strings;

	// Read properties to validate the savegame is for this game
	file_objects = read_integer(save);
	file_integers = read_integer(save);
	file_functions = read_integer(save);
	file_strings = read_integer(save);

	if (file_objects != objects
	        || file_integers != integers
	        || file_functions != functions
	        || file_strings != strings) {
		if (warn == FALSE) {
			log_error(cstring_resolve("BAD_SAVED_GAME")->value, PLUS_STDOUT);
		}
		g_vm->glk_stream_close(save, NULL);
		return (FALSE);
	}

	while (current_integer != NULL) {
		current_integer->value = read_integer(save);
		current_integer = current_integer->next_integer;
	}

	while (current_function != NULL) {
		current_function->call_count = read_integer(save);
		current_function = current_function->next_function;
	}

	for (index = 1; index <= objects; index++) {
		if (object[index]->nosave)
			continue;

		for (counter = 0; counter < 16; counter++) {
			object[index]->integer[counter] = read_integer(save);
		}

		object[index]->attributes = read_integer(save);
		object[index]->user_attributes = read_integer(save);
	}

	while (current_string != NULL) {
		for (index = 0; index < 255; index++) {
			current_string->value[index] = g_vm->glk_get_char_stream(save);
		}
		current_string = current_string->next_string;
	}

	player = read_integer(save);
	noun[3] = read_integer(save);

	// Restore the current volume of each of the sound channels
	for (index = 0; index < 8; index++) {
		sprintf(temp_buffer, "volume[%d]", index);
		counter = read_integer(save);
		cinteger_resolve(temp_buffer)->value = counter;

		if (SOUND_SUPPORTED->value) {
			// Set the GLK volume
			g_vm->glk_schannel_set_volume(sound_channel[index], (glui32) counter);
		}
	}

	// Restore the current value of the GLK timer
	counter = read_integer(save);
	cinteger_resolve("timer")->value = counter;

	// Set the GLK timer
	g_vm->glk_request_timer_events((glui32) counter);

	TIME->value = FALSE;
	return true;
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
