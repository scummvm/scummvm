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

#include "glk/alan2/alan2.h"
#include "glk/alan2/glkstart.h"
#include "glk/alan2/glkio.h"
#include "glk/alan2/args.h"

#include "glk/alan2/alan_version.h"

namespace Glk {
namespace Alan2 {

glkunix_argumentlist_t glkunix_arguments[] = {
	{ "-v", glkunix_arg_NoValue, "-v: verbose mode" },
	{ "-l", glkunix_arg_NoValue, "-l: log player command and game output" },
	{ "-i", glkunix_arg_NoValue, "-i: ignore version and checksum errors" },
	{ "-d", glkunix_arg_NoValue, "-d: enter debug mode" },
	{ "-t", glkunix_arg_NoValue, "-t: trace game execution" },
	{ "-s", glkunix_arg_NoValue, "-s: single instruction trace" },
	{ "", glkunix_arg_ValueFollows, "filename: The game file to load." },
	{ NULL, glkunix_arg_End, NULL }
};

bool glkunix_startup_code(glkunix_startup_t *data) {
	// first, open a window for error output
	glkMainWin = g_vm->glk_window_open(0, 0, 0, wintype_TextBuffer, 0); 
	if (glkMainWin == nullptr)
		error("FATAL ERROR: Cannot open initial window");

	g_vm->glk_stylehint_set(wintype_TextGrid, style_User1, stylehint_ReverseColor, 1);
	glkStatusWin = g_vm->glk_window_open(glkMainWin, winmethod_Above |
	winmethod_Fixed, 1, wintype_TextGrid, 0);
	g_vm->glk_set_window(glkMainWin);
  
	/* now process the command line arguments */
	args(data->argc, data->argv);

	g_vm->garglk_set_program_name(alan.shortHeader);
	g_vm->garglk_set_program_info("Alan Interpreter 2.8.6 by Thomas Nilsson\n");

	return true;
}

} // End of namespace Alan2
} // End of namespace Glk
