/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */
/*
 Description:	
 
	Command line parser

 Notes: 

*/

#include "reinherit.h"

namespace Saga {

int R_ReadCommandLine(int argc, char **argv, R_EXECINFO * execinfo)
{

	int arg;
	int intparam;

	for (arg = 1; arg < argc; arg++) {

		if (strcmp(argv[arg], "-scene") == 0) {
			arg++;
			if (arg <= argc) {
				intparam = atoi(argv[arg]);
				execinfo->start_scene = intparam;
				if (intparam == 0) {
					R_printf(R_STDERR,
					    "Error: Invalid parameter to '-scene'.\n");
					return -1;
				}
			} else {
				R_printf(R_STDERR,
				    "Error: '-scene' requires a parameter.\n");
				return -1;
			}
		}

		if (strcmp(argv[arg], "-gamedir") == 0) {
			arg++;
			if (arg <= argc) {

				execinfo->game_dir = argv[arg];
			} else {
				R_printf(R_STDERR,
				    "Error: '-gamedir' requires a parameter.\n");
				return -1;
			}
		}

		if (strcmp(argv[arg], "-noverify") == 0) {
			execinfo->no_verify = 1;
		}

		if (strcmp(argv[arg], "-nosound") == 0) {
			execinfo->no_sound = 1;
		}

		if (strcmp(argv[arg], "-nomusic") == 0) {
			execinfo->no_music = 1;
		}

		if (strcmp(argv[arg], "-windowed") == 0) {
			execinfo->windowed = 1;
		}

	}

	return 0;

}

} // End of namespace Saga
