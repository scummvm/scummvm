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

extern struct function_type     *executing_function;
extern const char               *word[];

extern char                     error_buffer[];

void badparrun() {
	sprintf(error_buffer, BAD_PARENT, executing_function->name);

	log_error(error_buffer, PLUS_STDERR);
}

void notintrun() {
	sprintf(error_buffer, NOT_INTEGER, executing_function->name, word[0]);
	log_error(error_buffer, PLUS_STDERR);
}

void unkfunrun(const char *name) {
	sprintf(error_buffer, UNKNOWN_FUNCTION_RUN, name);
	log_error(error_buffer, PLUS_STDOUT);
}

void unkkeyerr(int line, int wordno) {
	sprintf(error_buffer, UNKNOWN_KEYWORD_ERR, line, word[wordno]);
	log_error(error_buffer, PLUS_STDERR);
}

void unkatterr(int line, int wordno) {
	sprintf(error_buffer, UNKNOWN_ATTRIBUTE_ERR, line,
	        word[wordno]);
	log_error(error_buffer, PLUS_STDERR);
}

void unkvalerr(int line, int wordno) {
	sprintf(error_buffer, UNKNOWN_VALUE_ERR, line,
	        word[wordno]);
	log_error(error_buffer, PLUS_STDERR);
}

void noproprun(int) {
	sprintf(error_buffer, INSUFFICIENT_PARAMETERS_RUN, executing_function->name, word[0]);
	log_error(error_buffer, PLUS_STDOUT);
}

void noobjerr(int line) {
	sprintf(error_buffer, NO_OBJECT_ERR,
	        line, word[0]);
	log_error(error_buffer, PLUS_STDERR);
}

void noproperr(int line) {
	sprintf(error_buffer, INSUFFICIENT_PARAMETERS_ERR,
	        line, word[0]);
	log_error(error_buffer, PLUS_STDERR);
}

void nongloberr(int line) {
	sprintf(error_buffer, NON_GLOBAL_FIRST, line);
	log_error(error_buffer, PLUS_STDERR);
}

void nofnamerr(int line) {
	sprintf(error_buffer, NO_NAME_FUNCTION, line);
	log_error(error_buffer, PLUS_STDERR);
}

void unkobjerr(int line, int wordno) {
	sprintf(error_buffer, UNDEFINED_ITEM_ERR, line, word[wordno]);
	log_error(error_buffer, PLUS_STDERR);
}

void maxatterr(int line, int wordno) {
	sprintf(error_buffer,
	        MAXIMUM_ATTRIBUTES_ERR, line, word[wordno]);
	log_error(error_buffer, PLUS_STDERR);
}

void unkobjrun(int wordno) {
	sprintf(error_buffer, UNDEFINED_ITEM_RUN, executing_function->name, word[wordno]);
	log_error(error_buffer, PLUS_STDOUT);
}

void unkattrun(int wordno) {
	sprintf(error_buffer, UNKNOWN_ATTRIBUTE_RUN, executing_function->name, word[wordno]);
	log_error(error_buffer, PLUS_STDOUT);
}

void unkdirrun(int wordno) {
	sprintf(error_buffer, UNDEFINED_DIRECTION_RUN,
	        executing_function->name, word[wordno]);
	log_error(error_buffer, PLUS_STDOUT);
}

void badparun() {
	sprintf(error_buffer, BAD_PARENT, executing_function->name);
	log_error(error_buffer, PLUS_STDOUT);
}

void badplrrun(int value) {
	sprintf(error_buffer, BAD_PLAYER, executing_function->name, value);
	log_error(error_buffer, PLUS_STDOUT);
}

void badptrrun(const char *name, int value) {
	sprintf(error_buffer, BAD_POINTER, executing_function->name, name, value);
	log_error(error_buffer, PLUS_STDOUT);
}

void unkvarrun(const char *variable) {
	sprintf(error_buffer, UNDEFINED_CONTAINER_RUN, executing_function->name, arg_text_of(variable));
	log_error(error_buffer, PLUS_STDOUT);
}

void unkstrrun(const char *variable) {
	sprintf(error_buffer, UNDEFINED_STRING_RUN, executing_function->name, variable);
	log_error(error_buffer, PLUS_STDOUT);
}

void unkscorun(const char *scope) {
	sprintf(error_buffer, UNKNOWN_SCOPE_RUN, executing_function->name, scope);
	log_error(error_buffer, PLUS_STDOUT);
}

void totalerrs(int errors) {
	if (errors == 1)
		sprintf(error_buffer, ERROR_DETECTED);
	else {
		sprintf(error_buffer, ERRORS_DETECTED, errors);
	}

	log_error(error_buffer, PLUS_STDERR);
}

void outofmem() {
	log_error(OUT_OF_MEMORY, PLUS_STDERR);
	error("Terminated");
}

} // End of namespace JACL
} // End of namespace Glk
