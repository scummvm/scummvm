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
 * test_run_game_script()
 *
 * Run the game using the given script.  Returns the count of errors from
 * the script's monitoring.
 */
static sc_int test_run_game_script(sc_game game, sx_script script) {
	sc_int errors;

	/* Ensure completely repeatable random number sequences. */
	sc_reseed_random_sequence(1);

	/* Start interpreting the script stream. */
	scr_start_script(game, script);
	sc_interpret_game(game);

	/* Wrap up the script interpreter and capture error count. */
	errors = scr_finalize_script();
	return errors;
}


/*
 * test_run_game_tests()
 *
 * Run each test in the given descriptor array, reporting the results and
 * accumulating an error count overall.  Return the total error count.
 */
sc_int test_run_game_tests(const sx_test_descriptor_t tests[],
                           sc_int count, sc_bool is_verbose) {
	const sx_test_descriptor_t *test;
	sc_int errors;
	assert(tests);

	errors = 0;

	/* Execute each game in turn. */
	for (test = tests; test < tests + count; test++) {
		sc_int test_errors;

		if (is_verbose) {
			sx_trace("--- Running Test \"%s\" [\"%s\", by %s]...\n",
			         test->name,
			         sc_get_game_name(test->game),
			         sc_get_game_author(test->game));
		}

		test_errors = test_run_game_script(test->game, test->script);
		errors += test_errors;

		if (is_verbose) {
			sx_trace("--- Test \"%s\": ", test->name);
			if (test_errors > 0)
				sx_trace("%s [%ld error%s]\n",
				         "FAIL", test_errors, test_errors == 1 ? "" : "s");
			else
				sx_trace("%s\n", "PASS");
		} else
			sx_trace("%s", test_errors > 0 ? "F" : ".");
		//fflush (stdout);
		//fflush (stderr);
	}
	sx_trace("%s", is_verbose ? "" : "\n");

	return errors;
}

} // End of namespace Adrift
} // End of namespace Glk
