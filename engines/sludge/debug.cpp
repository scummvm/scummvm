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

#if 0
#include <stdarg.h>
#endif

#include "allfiles.h"
#include "debug.h"
#include "language.h"

void debugOut(const char *a, ...) {
	if (! gameSettings.debugMode) return;

	va_list argptr;
	va_start(argptr, a);

#if ALLOW_FILE
#if defined __unix__ && !(defined __APPLE__)
	vfprintf(stderr, a, argptr);
#else
	FILE *fp = fopen("debuggy.txt", "at");
	if (fp) {
		vfprintf(fp, a, argptr);
		fclose(fp);
	}
#endif
#endif
}

void debugHeader() {
	debugOut("*** Engine compiled " __DATE__ " at " __TIME__ ".\n");
}
