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
 * The glob matching functions in this module are derived from an original
 * (and somewhat hairy) glob.c posted by Arjan Kenter from the University
 * of Twente, NL, in an assortment of minor variations between 1993 and 1997.
 * The major modifications are:
 *
 *  o Added checks to ensure that invalid range patterns such as "[a-" or
 *    "[-" don't cause the loops to walk off the end of the pattern string
 *    and (usually) result in SIGSEGV.
 *  o Moved from plain char to unsigned char to avoid signedness problems
 *    with range comparisons.
 *  o Skipped the leading '[' in the range checker; the original was treating
 *    it as a possible first value of 'r'.
 *  o Moved the range checker while() from the bottom of the loop to the top,
 *    to avoid problems with invalid ranges.
 *  o Gave 'l' in the range checker an initial value that ensures that it
 *    can never match until it's been re-assigned to 'r'.
 *  o Used a return value rather than multiple returns in the matcher, for
 *    better debugability.
 *  o Applied some const-correctness, and replaced some pointers by indexing.
 *  o Added scanf-like special cases, making ']' a valid part of a range if
 *    first, and '-' if last.
 *
 * This glob accepts * and ? wild cards, and [] ranges.  It does not check
 * whether the range string is valid (for example, terminates with ']'), but
 * simply returns the best it can under those circumstances.
 *
 * Example call:
 *    glob_match ("a*b?c[A-Za-z_0-9]d*", some_string)
 */

/*
 * glob_inrange_unsigned()
 * glob_match_unsigned()
 *
 * Match a "[...]" character range, and match general glob wildcards.  See
 * above for notes on where these functions came from originally.
 */
static int glob_inrange_unsigned(const unsigned char **const pattern, unsigned char ch) {
	const unsigned char *const pattern_ = *pattern;
	int in_range = FALSE;
	unsigned int l = 256, r = 0, index_;

	/* Skip the leading '[' on entry to a range check. */
	index_ = 1;

	/* Special-case a range that has ']' as its first character. */
	if (pattern_[index_] == ']') {
		r = pattern_[index_++];
		if (ch == r)
			in_range = TRUE;
	}

	/*
	 * Check at the loop top, rather than the bottom, to avoid problems with
	 * invalid or uncompleted ranges.
	 */
	while (pattern_[index_] && pattern_[index_] != ']') {
		r = pattern_[index_++];
		if (r == '-') {
			/* Special-case a range that has '-' as its last character. */
			if (pattern_[index_] == ']' || !pattern_[index_]) {
				if (ch == r)
					in_range = TRUE;
				break;
			}

			/* Break the loop on unterminated range ending with '-'. */
			if (!pattern_[index_])
				break;

			r = pattern_[index_++];
			if (l <= ch && ch <= r)
				in_range = TRUE;
		} else {
			l = r;
			if (ch == r)
				in_range = TRUE;
		}
	}

	/* Update pattern with characters consumed, return result. */
	*pattern += index_;
	return in_range;
}

static int glob_match_unsigned(const unsigned char *pattern, const unsigned char *string) {
	int is_match = FALSE;

	if (!*string) {
		if (*pattern == '*')
			is_match = glob_match_unsigned(pattern + 1, string);
		else
			is_match = !*pattern;
	} else {
		switch (*pattern) {
		case '\0':
			is_match = !*string;
			break;
		case '*':
			if (glob_match_unsigned(pattern + 1, string))
				is_match = TRUE;
			else
				is_match = glob_match_unsigned(pattern, string + 1);
			break;
		case '?':
			is_match = glob_match_unsigned(pattern + 1, string + 1);
			break;
		case '[':
			/*
			 * After a range check, we need to see if we hit the end of the
			 * pattern before recursively matching pattern + 1.
			 */
			is_match = glob_inrange_unsigned(&pattern, *string)
			           && (!*pattern
			               || glob_match_unsigned(pattern + 1, string + 1));
			break;
		default:
			is_match = *pattern == *string
			           && glob_match_unsigned(pattern + 1, string + 1);
			break;
		}
	}

	return is_match;
}


/* Structures and data for the self test function. */
struct sx_test_data_t {
	const sc_char *const pattern;
	const sc_char *const string;
};

static const sx_test_data_t SHOULD_MATCH[] = {
	{"a", "a"}, {"abc", "abc"}, {"", ""},
	{"*", ""}, {"*", "abc"}, {"*", "cba"},
	{"*c", "c"}, {"*c", "abc"}, {"*c", "cbac"},
	{"a*", "a"}, {"a*", "abc"}, {"a*", "abca"},
	{"a*c", "ac"}, {"a*c", "abc"}, {"a*c", "abcbcbc"},
	{"a**c", "ac"}, {"a**c", "abc"}, {"a**c", "abcbcbc"},
	{"*b*", "b"}, {"*b*", "abc"}, {"*b*", "ab"}, {"*b*", "bc"},
	{"?", "a"}, {"?", "z"}, {"?", "?"}, {"[?]", "?"},
	{"a?", "aa"}, {"a?", "az"}, {"a?", "a?"},
	{"?c", "ac"}, {"?c", "zc"}, {"?c", "?c"},
	{"[abz]", "a"}, {"[abz]", "b"}, {"[abz]", "z"},
	{"[a-c]", "a"}, {"[a-c]", "b"}, {"[a-c]", "c"},
	{"[ac]b[ac]", "abc"}, {"[ac]b[ac]", "cba"},

	{"[]]", "]"}, {"[]a-c]", "a"}, {"[]a-c]", "b"}, {"[]a-c]", "c"},
	{"[?]", "?" }, {"[-]", "-"}, {"[z-]", "z"}, {"[z-]", "-"},
	{"[][-]", "]"}, {"[][-]", "["}, {"[][-]", "-"},
	{"[a-c-]", "a"}, {"[a-c-]", "b"}, {"[a-c-]", "c"}, {"[a-c-]", "-"},

	{"*[a-z]*abc?xyz", "a<star>abcQxyz"}, {"*[a-z]*abc?xyz", "<star>aabcQxyz"},
	{"*[a-z]*abc?xyz", "aabcQxyz"}, {"*[a-z]*abc?xyz", "<star>a<star>abcQxyz"},

	{"???]", "abc]"}, {"[z-a]", "z"},
	{"[a-z", "a"}, {"[a-", "a"}, {"[a", "a"}, {"[[", "["},
	{NULL, NULL}
};

static const sx_test_data_t SHOULD_NOT_MATCH[] = {
	{"a", "b"}, {"abc", "abd"}, {"a", ""}, {"", "a"},
	{"*c", "a"}, {"*c", "ab"}, {"*c", "abca"},
	{"a*", "c"}, {"a*", "cba"}, {"a*", "cbac"},
	{"a*c", "ca"}, {"a*c", "cba"}, {"a*c", "cbababa"},
	{"a**c", "ca"}, {"a**c", "cba"}, {"a**c", "cbababa"},
	{"*b*", ""}, {"*b*", "z"}, {"*b*", "ac"}, {"*b*", "azc"},
	{"?", ""}, {"?", "ab"}, {"?", "abc"}, {"[?]", "a"},
	{"a?", "ca"}, {"a?", "cz"}, {"a?", "??"},
	{"?c", "ab"}, {"?c", "zb"}, {"?c", "??"},
	{"[bcy]", "a"}, {"[bcy]", "d"}, {"[bcy]", "z"},
	{"[b-d]", "a"}, {"[b-d]", "e"}, {"[b-d]", ""}, {"[b-d]", "bc"},
	{"[ac]b[ac]", "aaa"}, {"[ac]b[ac]", "bbb"}, {"[ac]b[ac]", "ccc"},

	{"[]]", "["}, {"[]]", "a"}, {"[]a-c]", "z"},
	{"[?]", "a" }, {"[-]", "a"}, {"[z-]", "a"},
	{"[][-]", "a"}, {"[][-]", "z"},
	{"[a-c-]", "z"},

	{"*[a-z]*abc?xyz", "A<STAR>abcQxyz"}, {"*[a-z]*abc?xyz", "<STAR>AabcQxyz"},
	{"*[a-z]*abc?xyz", "AabcQxyz"}, {"*[a-z]*abc?xyz", "aabcxyz"},

	{"[z-a]", "a"}, {"[z-a]", "b"}, {"[", "a"}, {"[[", "a"},
	{NULL, NULL}
};


/*
 * glob_self_test()
 *
 * Sed quis custodiet ipsos custodes?
 */
static void glob_self_test(void) {
	const sx_test_data_t *test;
	sc_int errors;

	/*
	 * Run each test case and compare against expected result.  To avoid a lot
	 * of ugly casting, we use the main public glob_match() function.
	 */
	errors = 0;
	for (test = SHOULD_MATCH; test->pattern; test++) {
		if (!glob_match(test->pattern, test->string)) {
			sx_error("glob_self_test: \"%s\", \"%s\""
			         " did not match, and should have matched\n",
			         test->pattern, test->string);
			errors++;
		}
	}

	for (test = SHOULD_NOT_MATCH; test->pattern; test++) {
		if (glob_match(test->pattern, test->string)) {
			sx_error("glob_self_test: \"%s\", \"%s\""
			         " matched, and should not have matched\n",
			         test->pattern, test->string);
			errors++;
		}
	}

	/*
	 * Abort if any error.  As befits our distrustful nature, we won't even
	 * trust that sx_fatal() calls abort() (though it should).
	 */
	if (errors > 0) {
		sx_fatal("glob_self_test: %ld self-test error%s found, aborting\n",
		         errors, (errors == 1) ? "" : "s");
	}
}


/*
 * glob_match()
 *
 * Adapter for the above globbing functions, presenting a more standard char-
 * based interface.  Here is where all the evil casting lives.
 */
sc_bool glob_match(const sc_char *pattern, const sc_char *string) {
	static sc_bool initialized = FALSE;

	const unsigned char *pattern_ = (const unsigned char *) pattern;
	const unsigned char *string_ = (const unsigned char *) string;
	sc_bool retval;
	assert(pattern && string);

	/* On the first call, run a self-test to verify basic glob matching. */
	if (!initialized) {
		/*
		 * To avoid lots of icky casting, the self-test uses the core public
		 * glob_match() that we're in right here to run its tests.  So set
		 * initialized _before_ the test, to avoid infinite recursion.
		 */
		initialized = TRUE;
		glob_self_test();
	}

	retval = glob_match_unsigned(pattern_, string_) != 0;
	return retval;
}

} // End of namespace Adrift
} // End of namespace Glk
