/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
	Common::sprintf_s(error_buffer, 1024, BAD_PARENT, executing_function->name);

	log_error(error_buffer, PLUS_STDERR);
}

void notintrun() {
	Common::sprintf_s(error_buffer, 1024, NOT_INTEGER, executing_function->name, word[0]);
	log_error(error_buffer, PLUS_STDERR);
}

void unkfunrun(const char *name) {
	Common::sprintf_s(error_buffer, 1024, UNKNOWN_FUNCTION_RUN, name);
	log_error(error_buffer, PLUS_STDOUT);
}

void unkkeyerr(int line, int wordno) {
	Common::sprintf_s(error_buffer, 1024, UNKNOWN_KEYWORD_ERR, line, word[wordno]);
	log_error(error_buffer, PLUS_STDERR);
}

void unkatterr(int line, int wordno) {
	Common::sprintf_s(error_buffer, 1024, UNKNOWN_ATTRIBUTE_ERR, line,
	        word[wordno]);
	log_error(error_buffer, PLUS_STDERR);
}

void unkvalerr(int line, int wordno) {
	Common::sprintf_s(error_buffer, 1024, UNKNOWN_VALUE_ERR, line,
	        word[wordno]);
	log_error(error_buffer, PLUS_STDERR);
}

void noproprun(int) {
	Common::sprintf_s(error_buffer, 1024, INSUFFICIENT_PARAMETERS_RUN, executing_function->name, word[0]);
	log_error(error_buffer, PLUS_STDOUT);
}

void noobjerr(int line) {
	Common::sprintf_s(error_buffer, 1024, NO_OBJECT_ERR,
	        line, word[0]);
	log_error(error_buffer, PLUS_STDERR);
}

void noproperr(int line) {
	Common::sprintf_s(error_buffer, 1024, INSUFFICIENT_PARAMETERS_ERR,
	        line, word[0]);
	log_error(error_buffer, PLUS_STDERR);
}

void nongloberr(int line) {
	Common::sprintf_s(error_buffer, 1024, NON_GLOBAL_FIRST, line);
	log_error(error_buffer, PLUS_STDERR);
}

void nofnamerr(int line) {
	Common::sprintf_s(error_buffer, 1024, NO_NAME_FUNCTION, line);
	log_error(error_buffer, PLUS_STDERR);
}

void unkobjerr(int line, int wordno) {
	Common::sprintf_s(error_buffer, 1024, UNDEFINED_ITEM_ERR, line, word[wordno]);
	log_error(error_buffer, PLUS_STDERR);
}

void maxatterr(int line, int wordno) {
	Common::sprintf_s(error_buffer, 1024,
	        MAXIMUM_ATTRIBUTES_ERR, line, word[wordno]);
	log_error(error_buffer, PLUS_STDERR);
}

void unkobjrun(int wordno) {
	Common::sprintf_s(error_buffer, 1024, UNDEFINED_ITEM_RUN, executing_function->name, word[wordno]);
	log_error(error_buffer, PLUS_STDOUT);
}

void unkattrun(int wordno) {
	Common::sprintf_s(error_buffer, 1024, UNKNOWN_ATTRIBUTE_RUN, executing_function->name, word[wordno]);
	log_error(error_buffer, PLUS_STDOUT);
}

void unkdirrun(int wordno) {
	Common::sprintf_s(error_buffer, 1024, UNDEFINED_DIRECTION_RUN,
	        executing_function->name, word[wordno]);
	log_error(error_buffer, PLUS_STDOUT);
}

void badparun() {
	Common::sprintf_s(error_buffer, 1024, BAD_PARENT, executing_function->name);
	log_error(error_buffer, PLUS_STDOUT);
}

void badplrrun(int value) {
	Common::sprintf_s(error_buffer, 1024, BAD_PLAYER, executing_function->name, value);
	log_error(error_buffer, PLUS_STDOUT);
}

void badptrrun(const char *name, int value) {
	Common::sprintf_s(error_buffer, 1024, BAD_POINTER, executing_function->name, name, value);
	log_error(error_buffer, PLUS_STDOUT);
}

void unkvarrun(const char *variable) {
	Common::sprintf_s(error_buffer, 1024, UNDEFINED_CONTAINER_RUN, executing_function->name, arg_text_of(variable));
	log_error(error_buffer, PLUS_STDOUT);
}

void unkstrrun(const char *variable) {
	Common::sprintf_s(error_buffer, 1024, UNDEFINED_STRING_RUN, executing_function->name, variable);
	log_error(error_buffer, PLUS_STDOUT);
}

void unkscorun(const char *scope) {
	Common::sprintf_s(error_buffer, 1024, UNKNOWN_SCOPE_RUN, executing_function->name, scope);
	log_error(error_buffer, PLUS_STDOUT);
}

void totalerrs(int errors) {
	if (errors == 1)
		Common::sprintf_s(error_buffer, 1024, ERROR_DETECTED);
	else {
		Common::sprintf_s(error_buffer, 1024, ERRORS_DETECTED, errors);
	}

	log_error(error_buffer, PLUS_STDERR);
}

void outofmem() {
	log_error(OUT_OF_MEMORY, PLUS_STDERR);
	error("Terminated");
}

} // End of namespace JACL
} // End of namespace Glk
