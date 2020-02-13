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
#include "common/debug.h"
#include "common/str.h"
#include "common/file.h"
#include "common/textconsole.h"

namespace Glk {
namespace Adrift {

/*
 * sx_trace()
 *
 * Debugging trace function; printf wrapper that writes to stdout.  Note that
 * this differs from sc_trace(), which writes to stderr.  We use stdout so
 * that trace output is synchronized to test expectation failure messages.
 */
void sx_trace(const sc_char *format, ...) {
	va_list ap;
	assert(format);

	va_start(ap, format);
	Common::String line = Common::String::vformat(format, ap);
	va_end(ap);

	debug("%s", line.c_str());
}

/*
 * sx_error()
 * sx_fatal()
 *
 * Error reporting functions.  sx_error() prints a message and continues.
 * sx_fatal() prints a message, then calls abort().
 */
void sx_error(const sc_char *format, ...) {
	va_list ap;
	assert(format);

	va_start(ap, format);
	Common::String line = Common::String::vformat(format, ap);
	va_end(ap);

	warning("%s", line.c_str());
}

void sx_fatal(const sc_char *format, ...) {
	va_list ap;
	assert(format);

	va_start(ap, format);
	Common::String line = Common::String::vformat(format, ap);
	va_end(ap);

	error("%s", line.c_str());
}

/* Unique non-heap address for zero size malloc() and realloc() requests. */
static void *sx_zero_allocation = &sx_zero_allocation;

/*
 * sx_malloc()
 * sx_realloc()
 * sx_free()
 *
 * Non-failing wrappers around malloc functions.  Newly allocated memory is
 * cleared to zero.  In ANSI/ISO C, zero byte allocations are implementation-
 * defined, so we have to take special care to get predictable behavior.
 */
void *sx_malloc(size_t size) {
	void *allocated;

	if (size == 0)
		return sx_zero_allocation;

	allocated = malloc(size);
	if (!allocated)
		sx_fatal("sx_malloc: requested %lu bytes\n", (sc_uint) size);
	else if (allocated == sx_zero_allocation)
		sx_fatal("sx_malloc: zero-byte allocation address returned\n");

	memset(allocated, 0, size);
	return allocated;
}

void *sx_realloc(void *pointer, size_t size) {
	void *allocated;

	if (size == 0) {
		sx_free(pointer);
		return sx_zero_allocation;
	}

	if (pointer == sx_zero_allocation)
		pointer = NULL;

	allocated = realloc(pointer, size);
	if (!allocated)
		sx_fatal("sx_realloc: requested %lu bytes\n", (sc_uint) size);
	else if (allocated == sx_zero_allocation)
		sx_fatal("sx_realloc: zero-byte allocation address returned\n");

	if (!pointer)
		memset(allocated, 0, size);
	return allocated;
}

void sx_free(void *pointer) {
	if (sx_zero_allocation != &sx_zero_allocation)
		sx_fatal("sx_free: write to zero-byte allocation address detected\n");

	if (pointer && pointer != sx_zero_allocation)
		free(pointer);
}


/*
 * sx_fopen()
 *
 * Open a file for a given test name with the extension and mode supplied.
 * Returns NULL if unsuccessful.
 */
Common::SeekableReadStream *sx_fopen(const sc_char *name, const sc_char *extension, const sc_char *mode) {
	assert(name && extension && mode);

	Common::String filename = Common::String::format("%s.%s", name, extension);
	Common::File *f = new Common::File();

	if (f->open(filename))
		return f;

	delete f;
	return nullptr;
}


/* Miscellaneous general ascii constants. */
static const sc_char NUL = '\0';

/*
 * sx_isspace()
 * sx_isprint()
 *
 * Built in replacements for locale-sensitive libc ctype.h functions.
 */
static sc_bool sx_isspace(sc_char character) {
	static const sc_char *const WHITESPACE = "\t\n\v\f\r ";

	return character != NUL && strchr(WHITESPACE, character) != NULL;
}

static sc_bool sx_isprint(sc_char character) {
	static const sc_int MIN_PRINTABLE = ' ', MAX_PRINTABLE = '~';

	return character >= MIN_PRINTABLE && character <= MAX_PRINTABLE;
}


/*
 * sx_trim_string()
 *
 * Trim leading and trailing whitespace from a string.  Modifies the string
 * in place, and returns the string address for convenience.
 */
sc_char *sx_trim_string(sc_char *string) {
	sc_int index_;
	assert(string);

	for (index_ = strlen(string) - 1;
	        index_ >= 0 && sx_isspace(string[index_]); index_--)
		string[index_] = NUL;

	for (index_ = 0; sx_isspace(string[index_]);)
		index_++;
	memmove(string, string + index_, strlen(string) - index_ + 1);

	return string;
}


/*
 * sx_normalize_string()
 *
 * Trim a string, set all runs of whitespace to a single space character,
 * and convert all non-printing characters to '?'.  Modifies the string in
 * place, and returns the string address for convenience.
 */
sc_char *sx_normalize_string(sc_char *string) {
	sc_int index_;
	assert(string);

	string = sx_trim_string(string);

	for (index_ = 0; string[index_] != NUL; index_++) {
		if (sx_isspace(string[index_])) {
			sc_int cursor;

			string[index_] = ' ';
			for (cursor = index_ + 1; sx_isspace(string[cursor]);)
				cursor++;
			memmove(string + index_ + 1,
			        string + cursor, strlen(string + cursor) + 1);
		} else if (!sx_isprint(string[index_]))
			string[index_] = '?';
	}

	return string;
}

char *adrift_fgets(char *buf, int max, Common::SeekableReadStream *s) {
	char *ptr = buf;
	char c;
	while (s->pos() < s->size() && --max > 0) {
		c = s->readByte();
		if (c == '\n' || c == '\0')
			break;
		*ptr++ = c;
	}
	*ptr++ = '\0';
	return buf;
}

} // End of namespace Adrift
} // End of namespace Glk
