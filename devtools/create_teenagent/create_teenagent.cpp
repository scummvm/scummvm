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

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "util.h"
#include "static_tables.h"

int main(int argc, char *argv[]) {
	const char *dat_name = "teenagent.dat";

	FILE *fout = fopen(dat_name, "wb");
	if (fout == nullptr) {
		perror("opening output file");
		exit(1);
	}

	if (fwrite(cseg, CSEG_SIZE, 1, fout) != 1) {
		perror("Writing code segment");
		exit(1);
	}

	if (fwrite(dsegStartBlock, DSEG_STARTBLK_SIZE, 1, fout) != 1) {
		perror("Writing data segment start block");
		exit(1);
	}

	// Write out message string block
	for (uint i = 0; i < (sizeof(englishMessages)/sizeof(char*)); i++) {
		if (i == 0) {
			// Write out reject message pointer block
			uint16 off = DSEG_STARTBLK_SIZE + (4 * 2);
			writeUint16LE(fout, off);
			off += strlen(englishMessages[0]) + 2;
			writeUint16LE(fout, off);
			off += strlen(englishMessages[1]) + 2;
			writeUint16LE(fout, off);
			off += strlen(englishMessages[2]) + 2;
			writeUint16LE(fout, off);
		}

		if (i == 327) {
			// Write out book color pointer block
			uint16 off = DSEG_STARTBLK_SIZE + (4 * 2);
			for (uint k = 0; k < 327; k++)
				off += strlen(englishMessages[k]) + 2;
			off += (6 * 2);
			writeUint16LE(fout, off);
			off += strlen(englishMessages[327]) + 2;
			writeUint16LE(fout, off);
			off += strlen(englishMessages[328]) + 2;
			writeUint16LE(fout, off);
			off += strlen(englishMessages[329]) + 2;
			writeUint16LE(fout, off);
			off += strlen(englishMessages[330]) + 2;
			writeUint16LE(fout, off);
			off += strlen(englishMessages[331]) + 2;
			writeUint16LE(fout, off);
		}
		for (uint j = 0; j < strlen(englishMessages[i]); j++) {
			if (englishMessages[i][j] == '\n')
				writeByte(fout, '\0');
			else
				writeByte(fout, englishMessages[i][j]);
		}
		writeByte(fout, '\0');
		writeByte(fout, '\0');
	}

	if (fwrite(dsegEndBlock, DSEG_ENDBLK_SIZE, 1, fout) != 1) {
		perror("Writing data segment end block");
		exit(1);
	}

	uint currentFilePos = ftell(fout);
	uint prevFilePos = currentFilePos;
	uint32 resourceSize = 0;
	writeUint32LE(fout, resourceSize);

	// Write out dialog string block
	static const char nulls[6] = "\0\0\0\0\0";
	for (uint i = 0; i < (sizeof(englishDialogs)/sizeof(char**)); i++) {
		//printf("Writing Dialog #%d\n", i);
		bool dialogEnd = false;
		uint j = 0;
		while (!dialogEnd) {
			uint nullCount = 0;
			if (strcmp(englishDialogs[i][j], NEW_LINE) == 0) {
				nullCount = 1;
			} else if (strcmp(englishDialogs[i][j], DISPLAY_MESSAGE) == 0) {
				nullCount = 2;
			} else if (strcmp(englishDialogs[i][j], CHANGE_CHARACTER) == 0) {
				nullCount = 3;
			} else if (strcmp(englishDialogs[i][j], END_DIALOG) == 0) {
				nullCount = 4;
				dialogEnd = true;
			} else { // Deals with normal dialogue and ANIM_WAIT cases
				if (fwrite(englishDialogs[i][j], 1, strlen(englishDialogs[i][j]), fout) != strlen(englishDialogs[i][j])) {
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

	currentFilePos = ftell(fout);
	resourceSize = currentFilePos - prevFilePos - sizeof(uint32);
	fseek(fout, prevFilePos, SEEK_SET);
	writeUint32LE(fout, resourceSize);
	fseek(fout, currentFilePos, SEEK_SET);

	// Write out Items data
	currentFilePos = ftell(fout);
	prevFilePos = currentFilePos;
	writeUint32LE(fout, resourceSize);

	const char ***items = englishItems;
	const uint kNumInventoryItems = 92;

	for (uint i = 0; i < kNumInventoryItems; i++) {
		// Write item id
		writeByte(fout, i + 1);
		// Write animated flag
		if (i == 6 || i == 13 || i == 47 || i == 49 || i == 67 || i == 91)
			writeByte(fout, 0x01);
		else
			writeByte(fout, 0x00);

		// Write name and description (if exists) of an item
		uint j = 0;
		bool endItem = false;
		while (!endItem) {
			if (strcmp(items[i][j], "\n") == 0) { // Separator between name and description
				writeByte(fout, '\0');
			} else if (strcmp(items[i][j], "\n\n") == 0) {
				writeByte(fout, '\0');
				writeByte(fout, '\0');
				endItem = true;
			} else {
				if (fwrite(items[i][j], 1, strlen(items[i][j]), fout) != strlen(items[i][j])) {
					perror("Writing item string");
					exit(1);
				}
			}

			j++;
		}
	}

	currentFilePos = ftell(fout);
	resourceSize = currentFilePos - prevFilePos - sizeof(uint32);
	fseek(fout, prevFilePos, SEEK_SET);
	writeUint32LE(fout, resourceSize);
	fseek(fout, currentFilePos, SEEK_SET);

	// Write out Credits data
	currentFilePos = ftell(fout);
	prevFilePos = currentFilePos;
	writeUint32LE(fout, resourceSize);

	Common::Array<const char *> &credits = englishCredits;

	for (auto &creditStr : credits) {
		for (uint j = 0; j < strlen(creditStr); j++) {
			if (creditStr[j] == '\n') {
				writeByte(fout, '\0');
			} else {
				writeByte(fout, creditStr[j]);
			}
		}
	}

	currentFilePos = ftell(fout);
	resourceSize = currentFilePos - prevFilePos - sizeof(uint32);
	fseek(fout, prevFilePos, SEEK_SET);
	writeUint32LE(fout, resourceSize);
	fseek(fout, currentFilePos, SEEK_SET);

	fclose(fout);

	return 0;
}
