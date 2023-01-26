/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <kos32sys.h>
#include <stdio.h>

/* This is just a small wrapper so that the main scummvm is loaded as dll.  */
int kolibrios_main(int argc, char *argv[]);

int main(int argc, char *argv[]) {
	printf("Loading SCUMMVM\n");
	if (argc < 1 || strnlen(argv[0], 3005) > 3000) {
		fprintf(stderr, "Couldn't determine exe path");
		return 1;
	}
	const char *r = strrchr(argv[0], '/');
	static char dllName[4000];
	int p = 0;
	if (r) {
		p = r - argv[0] + 1;
		memcpy(dllName, argv[0], p);
	}
	memcpy(dllName + p, "scummvm.dll", sizeof("scummvm.dll"));
	
	void *dlHandle = load_library(dllName);
	if (!dlHandle) {
		fprintf(stderr, "Couldn't load %s", dllName);
		return 2;
	}

	void (*kolibrios_main) (int argc, char *argv[]) = get_proc_address(dlHandle, "kolibrios_main");
	if (!kolibrios_main) {
		fprintf(stderr, "Failed to located kolibrios_main");
		return 3;
	}

	kolibrios_main(argc, argv);
}
