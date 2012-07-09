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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

// HACK to allow building with the SDL backend on MinGW
// see bug #1800764 "TOOLS: MinGW tools building broken"
#ifdef main
#undef main
#endif // main

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "util.h"
#include "static_tables.h"

int main(int argc, char *argv[]) {
	const char *dat_name = "teenagent.dat";

	FILE *fout = fopen(dat_name, "wb");
	if (fout == NULL) {
		perror("opening output file");
		exit(1);
	}

	if (fwrite(cseg, CSEG_SIZE, 1, fout) != 1) {
		perror("Writing code segment");
		exit(1);
	}

	if (fwrite(dseg, DSEG_SIZE, 1, fout) != 1) {
		perror("Writing data segment");
		exit(1);
	}

	// Write out dialog string block
	static const char nulls[6] = "\0\0\0\0\0";
	for (uint i = 0; i < (sizeof(dialogs)/sizeof(char**)); i++) {
		//printf("Writing Dialog #%d\n", i);
		bool dialogEnd = false;
		uint j = 0;
		while (!dialogEnd) {
			uint nullCount = 0;
			if (strcmp(dialogs[i][j], NEW_LINE) == 0) {
				nullCount = 1;
			} else if (strcmp(dialogs[i][j], DISPLAY_MESSAGE) == 0) {
				nullCount = 2;
			} else if (strcmp(dialogs[i][j], CHANGE_CHARACTER) == 0) {
				nullCount = 3;
			} else if (strcmp(dialogs[i][j], END_DIALOG) == 0) {
				nullCount = 4;
				dialogEnd = true;
			} else { // Deals with normal dialogue and ANIM_WAIT cases
				if (fwrite(dialogs[i][j], 1, strlen(dialogs[i][j]), fout) != strlen(dialogs[i][j])) {
					perror("Writing dialog string");
					exit(1);
				}
			}

			if (nullCount != 0 && nullCount < 5) {
				if (fwrite(nulls, 1, nullCount, fout) != nullCount) {
					perror("Writing dialog string nulls");
					exit(1);
				}
			}

			j++;
		}
	}

	fclose(fout);

	fprintf(stderr, "please run \"gzip -n %s\"\n", dat_name);

	return 0;
}
