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
#include "common/textconsole.h"

namespace Glk {
namespace Adrift {

/*
 * Module notes:
 *
 * o In order to validate all its arguments, this module creates and retains
 *   a copy of every valid game encountered, only freeing them all at the end.
 *   This could make it very memory-hungry when running a large number of
 *   games and scripts.  The alternative is to only create games as needed,
 *   and free them once used, but it's nice to fully validate all command line
 *   arguments first, and work afterwards, so for now that's what's done.
 *
 * o ...Alternatively, we could validate by creating the game, destroy it,
 *   and then re-parse it later when running the test script.  Unfortunately,
 *   parsing an Adrift game can be lengthy (~seconds), so paying this price
 *   twice isn't too attractive either.
 *
 * o For now, then, if running lots of test, run in batches of ten or so.
 */

/*
 * main()
 *
 * Validate the command line, and each argument as a game to be run.
 * Execute scripts for each, and return with an error code if any test fails.
 */
int glk_main(int argc, const char *argv[]) {
	const sc_char *const program = argv[0];
	sc_bool is_verbose = FALSE, is_tracing = FALSE;
	const sc_char *trace_flags;
	sx_test_descriptor_t *tests;
	sc_int count, index_, errors;
	assert(argc > 0 && argv);

	/* Get options and validate the command line. */
	if (argc > 1
	        && (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "-vv") == 0)) {
		is_verbose = TRUE;
		is_tracing = (strcmp(argv[1], "-vv") == 0);
		argc--;
		argv++;
	}

	if (is_verbose) {
		sx_trace("--- %s Test Suite [Adrift %ld compatible]\n",
		         sc_scare_version(), sc_scare_emulation());
		if (argc < 2)
			return EXIT_SUCCESS;
	} else if (argc < 2) {
		error("Usage: %s [-v | -vv] test [test...]\n", program);
		return EXIT_FAILURE;
	}

	/* Ensure that the interpreter is in the Latin1 locale, and stays there. */
	if (!sc_set_locale("Latin1")) {
		error("%s: failed to set locale\n", program);
		return EXIT_FAILURE;
	}

	/*
	 * Force test reproducibility.  Because game construction may use random
	 * numbers, we also need to remember to reseed this before constructing
	 * each game, and then again before running each.
	 */
	sc_set_portable_random(TRUE);

	/* Set verbosity and tracing for other modules. */
	scr_set_verbose(is_verbose);
	stub_debug_trace(is_tracing);
	trace_flags = 0; // getenv("SC_TRACE_FLAGS");
	if (trace_flags)
		sc_set_trace_flags(strtoul(trace_flags, NULL, 0));

	/* Create an array of test descriptors large enough for all tests. */
	tests = (sx_test_descriptor_t *)sx_malloc((argc - 1) * sizeof(*tests));

	/* Validate each test argument by opening a game and script for it. */
	count = 0;
	for (index_ = 1; index_ < argc; index_++) {
		const sc_char *name;
		Common::SeekableReadStream *stream;
		sx_script script;
		sc_game game;

		name = argv[index_];

		script = sx_fopen(name, "scr", "r");
		if (!script) {
			error("%s: %s.scr: %s\n", program, name, "open");
			continue;
		}

		stream = sx_fopen(name, "taf", "rb");
		if (!stream) {
			error("%s: %s.taf: %s\n", program, name, "open");
			delete script;
			continue;
		}

		sc_reseed_random_sequence(1);
		game = sc_game_from_stream(stream);
		delete stream;
		if (!game) {
			error("%s: %s.taf: Unable to decode Adrift game\n", program, name);
			delete script;
			continue;
		}

		tests[count].name = name;
		tests[count].script = script;
		tests[count].game = game;
		count++;
	}

	/* Run the available tests and report results. */
	if (count > 0)
		errors = test_run_game_tests(tests, count, is_verbose);
	else
		errors = 1;

	/* Clean up allocations and opened files. */
	for (index_ = 0; index_ < count; index_++) {
		delete tests[index_].script;
		sc_free_game(tests[index_].game);
	}
	sx_free(tests);

	/* Report results overall. */
	warning("%s [%ld test%s, %ld error%s]\n",
	        errors > 0 ? "FAIL" : "PASS",
	        count, count == 1 ? "" : "s", errors, errors == 1 ? "" : "s");

	return errors > 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}

} // End of namespace Adrift
} // End of namespace Glk
