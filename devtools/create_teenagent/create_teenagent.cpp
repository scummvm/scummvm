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
/*
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

	if (fwrite(eseg, ESEG_SIZE, 1, fout) != 1) {
		perror("Writing second data segment");
		exit(1);
	}

	fclose(fout);

	fprintf(stderr, "please run \"gzip -n %s\"\n", dat_name);
*/

	printf("#define ANIM_WAIT        \"\\xff\"\n");
	printf("#define NEW_LINE         \"\\n\"\n");
	printf("#define DISPLAY_MESSAGE  \"\\n\\n\"\n");
	printf("#define CHANGE_CHARACTER \"\\n\\n\\n\"\n");
	printf("#define END_DIALOG       \"\\n\\n\\n\\n\"\n");
	printf("\n");
	int dialog_num = 0;
	printf("const static char* dialog_%d[] = {\n", dialog_num);
	int n = 0;
	uint8 last = 0xff;
	for (int i = 0; i < ESEG_SIZE; i++) {
		if ((eseg[i] != 0x00 && last == 0x00) || n == 4) {
			switch (n) {
				case 1:
					printf("\tNEW_LINE,\n");
					break;
				case 2:
					printf("\tDISPLAY_MESSAGE,\n");
					break;
				case 3:
					printf("\tCHANGE_CHARACTER,\n");
					break;
				case 4:
					dialog_num++;
					printf("\tEND_DIALOG\n};\n\n");
					printf("const static char* dialog_%d[] = {\n", dialog_num);
					break;
				default:
					fprintf(stderr, "ERROR: %d is too many nulls\n", n);
					break;
			}
			n = 0;
		}

		switch (eseg[i]) {
			case 0x00:
				n++;
				if (!(last == 0x00 || last == 0xff))
					printf("\",\n");
				break;
			case 0xff:
				if (!(last == 0x00 || last == 0xff))
					printf("\",\n");
				printf("\tANIM_WAIT,\n");
				break;
			default:
				if ((last == 0x00 || last == 0xff))
					printf("\t\"");

				if (eseg[i] >= 32 && eseg[i] <= 126) {
					if (eseg[i] == '\"')
						printf("\\");
					printf("%c", eseg[i]);
				} else fprintf(stderr, "ERROR: eseg[%d]:0x%02x is outside range\n", i, eseg[i]);
				break;
		}

		last = eseg[i];
	}

	if (n == 4) {
		dialog_num++;
		printf("\tEND_DIALOG\n};\n");
	} else fprintf(stderr, "ERROR: premature end of data\n");

	printf("\nconst static char** dialogs[] = {\n");
	for (int i = 0; i < dialog_num; i++)
		printf("\tdialog_%d%s\n", i, (i == dialog_num-1) ? "" : ",");
	printf("};\n");

	return 0;
}
