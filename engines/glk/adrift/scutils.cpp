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
#include "glk/glk.h"
#include "glk/events.h"
#include "common/debug.h"
#include "common/str.h"
#include "common/textconsole.h"

namespace Glk {
namespace Adrift {

/*
 * Module notes:
 *
 * o Implement smarter selective module tracing.
 */

/*
 * sc_trace()
 *
 * Debugging trace function; printf wrapper that writes to stderr.
 */
void sc_trace(const sc_char *format, ...) {
	va_list ap;
	assert(format);

	va_start(ap, format);
	Common::String s = Common::String::format(format, ap);
	va_end(ap);
	debug("%s", s.c_str());
}


/*
 * sc_error()
 * sc_fatal()
 *
 * Error reporting functions.  sc_error() prints a message and continues.
 * sc_fatal() prints a message, then calls abort().
 */
void sc_error(const sc_char *format, ...) {
	va_list ap;
	assert(format);

	va_start(ap, format);
	Common::String s = Common::String::vformat(format, ap);
	va_end(ap);
	warning("%s", s.c_str());
}

void sc_fatal(const sc_char *format, ...) {
	va_list ap;
	assert(format);

	va_start(ap, format);
	Common::String s = Common::String::format(format, ap);
	va_end(ap);
	error("%s", s.c_str());
}


/* Unique non-heap address for zero size malloc() and realloc() requests. */
static void *sc_zero_allocation = &sc_zero_allocation;

/*
 * sc_malloc()
 * sc_realloc()
 * sc_free()
 *
 * Non-failing wrappers around malloc functions.  Newly allocated memory is
 * cleared to zero.  In ANSI/ISO C, zero byte allocations are implementation-
 * defined, so we have to take special care to get predictable behavior.
 */
void *sc_malloc(size_t size) {
	void *allocated;

	if (size == 0)
		return sc_zero_allocation;

	allocated = malloc(size);
	if (!allocated)
		sc_fatal("sc_malloc: requested %lu bytes\n", (sc_uint) size);
	else if (allocated == sc_zero_allocation)
		sc_fatal("sc_malloc: zero-byte allocation address returned\n");

	memset(allocated, 0, size);
	return allocated;
}

void *sc_realloc(void *pointer, size_t size) {
	void *allocated;

	if (size == 0) {
		sc_free(pointer);
		return sc_zero_allocation;
	}

	if (pointer == sc_zero_allocation)
		pointer = NULL;

	allocated = realloc(pointer, size);
	if (!allocated)
		sc_fatal("sc_realloc: requested %lu bytes\n", (sc_uint) size);
	else if (allocated == sc_zero_allocation)
		sc_fatal("sc_realloc: zero-byte allocation address returned\n");

	if (!pointer)
		memset(allocated, 0, size);
	return allocated;
}

void sc_free(void *pointer) {
	if (sc_zero_allocation != &sc_zero_allocation)
		sc_fatal("sc_free: write to zero-byte allocation address detected\n");

	if (pointer && pointer != sc_zero_allocation)
		free(pointer);
}


/*
 * sc_strncasecmp()
 * sc_strcasecmp()
 *
 * Strncasecmp and strcasecmp are not ANSI functions, so here are local
 * definitions to do the same jobs.
 */
sc_int sc_strncasecmp(const sc_char *s1, const sc_char *s2, sc_int n) {
	sc_int index_;
	assert(s1 && s2);

	for (index_ = 0; index_ < n; index_++) {
		sc_int diff;

		diff = sc_tolower(s1[index_]) - sc_tolower(s2[index_]);
		if (diff < 0 || diff > 0)
			return diff < 0 ? -1 : 1;
	}

	return 0;
}

sc_int sc_strcasecmp(const sc_char *s1, const sc_char *s2) {
	sc_int s1len, s2len, result;
	assert(s1 && s2);

	s1len = strlen(s1);
	s2len = strlen(s2);

	result = sc_strncasecmp(s1, s2, s1len < s2len ? s1len : s2len);
	if (result < 0 || result > 0)
		return result;
	else
		return s1len < s2len ? -1 : s1len > s2len ? 1 : 0;
}


/*
 * sc_platform_rand()
 * sc_congruential_rand()
 * sc_set_random_handler()
 *
 * Internal random number generation functions.  We offer two: one is a self-
 * seeding wrapper around the platform's rand(), which should generate good
 * random numbers but with a sequence that is platform-dependent; the other
 * is a linear congruential generator with a long period that is guaranteed
 * to return the same sequence for all platforms.  The default is the first,
 * with the latter intended for predictability of game actions.
 */
static sc_int sc_platform_rand(sc_uint new_seed) {
	static sc_bool is_seeded = FALSE;

	/* If reseeding, seed with the value supplied, note seeded, and return 0. */
	if (new_seed > 0) {
		g_vm->setRandomNumberSeed(new_seed);
		is_seeded = TRUE;
		return 0;
	} else {
		/* If not explicitly seeded yet, generate a seed from time(). */
		if (!is_seeded) {
			//srand ((sc_uint) time (NULL));
			is_seeded = TRUE;
		}

		/* Return the next rand() number in the sequence. */
		return g_vm->getRandomNumber(0xffffff);
	}
}

static sc_int sc_congruential_rand(sc_uint new_seed) {
	static sc_bool is_seeded = FALSE;
	static sc_uint rand_state = 1;

	/* If reseeding, seed with the value supplied, and note seeded. */
	if (new_seed > 0) {
		rand_state = new_seed;
		is_seeded = TRUE;
		return 0;
	} else {
		/* If not explicitly seeded yet, generate a seed from time(). */
		if (!is_seeded) {
			rand_state = (sc_uint)g_vm->_events->getTotalPlayTicks();
			is_seeded = TRUE;
		}

		/*
		 * Advance random state, using constants from Park & Miller (1988).
		 * To keep the values the same for both 32 and 64 bit longs, mask out
		 * any bits above the bottom 32.
		 */
		rand_state = (rand_state * 16807 + 2147483647) & 0xffffffff;

		/*
		 * Discard the lowest bit as a way to map 32-bits unsigned to a 32-bit
		 * positive signed.
		 */
		return rand_state >> 1;
	}
}


/* Function pointer for the actual random number generator in use. */
static sc_int(*sc_rand_function)(sc_uint) = sc_platform_rand;

/*
 * sc_set_congruential_random()
 * sc_set_platform_random()
 * sc_is_congruential_random()
 * sc_seed_random()
 * sc_rand()
 * sc_randomint()
 *
 * Public interface to random functions; control and reseed the random
 * handler in use, generate a random number, and a convenience function to
 * generate a random value within a given range.
 */
void sc_set_congruential_random(void) {
	sc_rand_function = sc_congruential_rand;
}

void sc_set_platform_random(void) {
	sc_rand_function = sc_platform_rand;
}

sc_bool sc_is_congruential_random(void) {
	return sc_rand_function == sc_congruential_rand;
}

void sc_seed_random(sc_uint new_seed) {
	/* Ignore zero values of new_seed by simply using 1 instead. */
	sc_rand_function(new_seed > 0 ? new_seed : 1);
}

sc_int sc_rand(void) {
	sc_int retval;

	/* Passing zero indicates this is not a seed operation. */
	retval = sc_rand_function(0);
	return retval;
}

sc_int sc_randomint(sc_int low, sc_int high) {
	/*
	 * If the range is invalid, just return the low value given.  This mimics
	 * Adrift under the same conditions, and also guards against division by
	 * zero in the mod operation.
	 */
	return (high < low) ? low : low + sc_rand() % (high - low + 1);
}


/* Miscellaneous general ascii constants. */
static const sc_char NUL = '\0';
static const sc_char SPACE = ' ';

/*
 * sc_strempty()
 *
 * Return TRUE if a string is either zero-length or contains only whitespace.
 */
sc_bool sc_strempty(const sc_char *string) {
	sc_int index_;
	assert(string);

	/* Scan for any non-space character. */
	for (index_ = 0; string[index_] != NUL; index_++) {
		if (!sc_isspace(string[index_]))
			return FALSE;
	}

	/* None found, so string is empty. */
	return TRUE;
}


/*
 * sc_trim_string()
 *
 * Trim leading and trailing whitespace from a string.  Modifies the string
 * in place, and returns the string address for convenience.
 */
sc_char *sc_trim_string(sc_char *string) {
	sc_int index_;
	assert(string);

	for (index_ = strlen(string) - 1;
	        index_ >= 0 && sc_isspace(string[index_]); index_--)
		string[index_] = NUL;

	for (index_ = 0; sc_isspace(string[index_]);)
		index_++;
	memmove(string, string + index_, strlen(string) - index_ + 1);

	return string;
}


/*
 * sc_normalize_string()
 *
 * Trim a string, and set all runs of whitespace to a single space character.
 * Modifies the string in place, and returns the string address for
 * convenience.
 */
sc_char *sc_normalize_string(sc_char *string) {
	sc_int index_;
	assert(string);

	/* Trim all leading and trailing spaces. */
	string = sc_trim_string(string);

	/* Compress multiple whitespace runs into a single space character. */
	for (index_ = 0; string[index_] != NUL; index_++) {
		if (sc_isspace(string[index_])) {
			sc_int cursor;

			string[index_] = SPACE;
			for (cursor = index_ + 1; sc_isspace(string[cursor]);)
				cursor++;
			memmove(string + index_ + 1,
			        string + cursor, strlen(string + cursor) + 1);
		}
	}

	return string;
}


/*
 * sc_compare_word()
 *
 * Return TRUE if the first word in the string is word, case insensitive.
 */
sc_bool sc_compare_word(const sc_char *string, const sc_char *word, sc_int length) {
	assert(string && word);

	/* Return TRUE if string starts with word, then space or string end. */
	return sc_strncasecmp(string, word, length) == 0
	       && (string[length] == NUL || sc_isspace(string[length]));
}


/*
 * sc_hash()
 *
 * Hash a string, hashpjw algorithm, from 'Compilers, principles, techniques,
 * and tools', page 436, unmodulo'ed and somewhat restyled.
 */
sc_uint sc_hash(const sc_char *string) {
	sc_int index_;
	sc_uint hash;
	assert(string);

	hash = 0;
	for (index_ = 0; string[index_] != NUL; index_++) {
		sc_uint temp;

		hash = (hash << 4) + string[index_];
		temp = hash & 0xf0000000;
		if (temp != 0) {
			hash = hash ^ (temp >> 24);
			hash = hash ^ temp;
		}
	}

	return hash;
}

} // End of namespace Adrift
} // End of namespace Glk
