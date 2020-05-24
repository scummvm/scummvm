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
#include "glk/alan2/args.h"
#include "glk/alan2/glkio.h"
#include "glk/alan2/main.h"
#include "glk/alan2/sysdep.h"

namespace Glk {
namespace Alan2 {

static void switches(unsigned argc, char *argv[]) {
}

void args(int argc, char *argv[]) {
	char *prgnam;

	if ((prgnam = strrchr(argv[0], '\\')) == NULL
	        && (prgnam = strrchr(argv[0], '/')) == NULL
	        && (prgnam = strrchr(argv[0], ':')) == NULL)
		prgnam = argv[0];
	else
		prgnam++;
	if (strlen(prgnam) > 4
	        && (strcmp(&prgnam[strlen(prgnam) - 4], ".EXE") == 0
	            || strcmp(&prgnam[strlen(prgnam) - 4], ".exe") == 0))
		prgnam[strlen(prgnam) - 4] = '\0';
	/* Now look at the switches and arguments */
	switches(argc, argv);
	if (advnam[0] == '\0')
		/* No game given, try program name */
		if (scumm_stricmp(prgnam, PROGNAME) != 0
		        && strstr(prgnam, PROGNAME) == 0)
			advnam = scumm_strdup(argv[0]);
}

} // End of namespace Alan2
} // End of namespace Glk
