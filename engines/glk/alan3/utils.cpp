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

#include "glk/alan3/utils.h"
#include "glk/alan3/alan_version.h"
#include "glk/alan3/glkio.h"
#include "glk/alan3/options.h"
#include "glk/alan3/memory.h"
#include "glk/alan3/output.h"
#include "glk/alan3/exe.h"
#include "glk/alan3/state.h"
#include "glk/alan3/lists.h"
#include "glk/alan3/fnmatch.h"

namespace Glk {
namespace Alan3 {

/*======================================================================

  terminate()

  Terminate the execution of the adventure, e.g. close windows,
  return buffers...

 */
void terminate(CONTEXT, int code) {
	newline();

	terminateStateStack();

	stopTranscript();

	if (memory) {
		deallocate(memory);
		memory = nullptr;
	}

	g_io->glk_exit();
	LONG_JUMP
}

/*======================================================================*/
void printVersion(int buildNumber) {
	printf("Arun - Adventure Language Interpreter version %s", alan.version.string);
	if (buildNumber != 0) printf("-%d", buildNumber);
	printf(" (%s %s)", alan.date, alan.time);
}


/*======================================================================*/
void usage(const char *programName) {
#if (BUILD+0) != 0
	printVersion(BUILD);
#else
	printVersion(0);
#endif
	printf("\n\nUsage:\n\n");
	printf("    %s [<switches>] <adventure>\n\n", programName);
	printf("where the possible optional switches are:\n");

	g_io->glk_set_style(style_Preformatted);
	printf("    -v       verbose mode\n");
	printf("    -l       log transcript to a file\n");
	printf("    -c       log player commands to a file\n");
	printf("    -n       no Status Line\n");
	printf("    -d       enter debug mode\n");
	printf("    -t[<n>]  trace game execution, higher <n> gives more trace\n");
	printf("    -i       ignore version and checksum errors\n");
	printf("    -r       make regression test easier (don't timestamp, page break, randomize...)\n");
	g_io->glk_set_style(style_Normal);
}


#ifndef FNM_CASEFOLD
#define FNM_CASEFOLD 0
#endif
/*======================================================================*/
bool match(const char *pattern, char *input) {
	return fnmatch(pattern, input, FNM_CASEFOLD) == 0;
}

} // End of namespace Alan3
} // End of namespace Glk
