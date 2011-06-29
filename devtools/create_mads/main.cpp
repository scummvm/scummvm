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

// HACK to allow building with the SDL backend on MinGW
// see bug #1800764 "TOOLS: MinGW tools building broken"
#ifdef main
#undef main
#endif // main

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"

#define BUFFER_SIZE 8192

void link(const char *destFilename, char **srcFilenames, int srcCount) {
	if (srcCount <= 0)
		return;

	FILE *destFile = fopen(destFilename, "wb");
	if (!destFile)
		return;
	unsigned int v = 0;
	const char *headerStr = "MADS";
	int fileOffset = 4 * (srcCount + 2);

	// Write header bit
	fwrite(headerStr, 1, 4, destFile);
	for (int i = 0; i <= srcCount; ++i)
		fwrite(&v, 1, 4, destFile);

	// Set up buffer for copying
	void *tempBuffer = malloc(BUFFER_SIZE);

	// Loop through copying each source file and setting it's file offset in the header
	for (int i = 0; i < srcCount; ++i) {
		// Add any extra padding to ensure that each file starts on a paragraph boundary
		if ((fileOffset % 16) != 0) {
			v = 0;
			while ((fileOffset % 16) != 0) {
				fwrite(&v, 1, 1, destFile);
				++fileOffset;
			}
		}

		FILE *srcFile = fopen(srcFilenames[i], "rb");
		if (!srcFile) {
			printf("Could not locate file '%s'\n", srcFilenames[i]);
			break;
		}

		// Set the starting position of the file
		fseek(destFile, 4 + (i * 4), SEEK_SET);
		fwrite(&fileOffset, 1, 4, destFile);

		// Move back to the end of the destination and copy the source file contents over
		fseek(destFile, 0, SEEK_END);
		while (!feof(srcFile)) {
			int bytesRead = fread(tempBuffer, 1, BUFFER_SIZE, srcFile);
			fwrite(tempBuffer, 1, bytesRead, destFile);
			fileOffset += bytesRead;
		}

		fclose(srcFile);
	}

	fclose(destFile);
	free(tempBuffer);
	printf("Done.\n");
}

int main(int argc, char *argv[]) {
	if (argc == 1) {
		printf("%s - ScummVM MADS Game script compiler v 1.0\n\n", argv[0]);
		printf("Parameters: %s src_filename.txt [dest_filename.bin] - Compiles a script text file to an output binary\t", argv[0]);
		printf("\t%s /link mads.dat filename1.bin [filename2.bin ..] - Joins one or more compiled Bin files to make\n", argv[0]);
		printf("an output suitable for running in ScummVM.\n\n");
	} else if (!strcmp(argv[1], "/link")) {
		// Link intermediate files into a final mads.dat file
		if (argc < 4)
			printf("Insufficient parameters\n");
		else
			link(argv[2], &argv[3], argc - 3);

	} else {
		// Compile a file
		char buffer[256];
		const char *destFilename = buffer;
		if (argc >= 3)
			destFilename = argv[2];
		else {
			// Use the source filename, but change the extension to '.bin'
			strcpy(buffer, argv[1]);
			char *p = buffer + strlen(buffer) - 1;
			while ((p >= buffer) && (*p != '.')) --p;
			if (p > buffer)
				// Change the extension
				strcpy(p, ".bin");
		}

		// Compile the specified source file
		bool result = Compile(argv[1], destFilename);
		return result ? 0 : 1;
	}

	return 0;
}