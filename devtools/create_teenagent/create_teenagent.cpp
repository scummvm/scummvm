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
#include "create_teenagent.h"
#include "static_tables.h"

void writeCredits(FILE *fd) {
	Common::Array<const char *> &credits = englishCredits;

	for (auto &creditStr : credits) {
		for (uint j = 0; j < strlen(creditStr); j++) {
			if (creditStr[j] == '\n') {
				writeByte(fd, '\0');
			} else {
				writeByte(fd, creditStr[j]);
			}
		}
	}
}

void writeDialogs(FILE *fd) {
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
				if (fwrite(englishDialogs[i][j], 1, strlen(englishDialogs[i][j]), fd) != strlen(englishDialogs[i][j])) {
					perror("Writing dialog string");
					exit(1);
				}
			}

			if (nullCount != 0 && nullCount < 5) {
				if (fwrite(nulls, 1, nullCount, fd) != nullCount) {
					perror("Writing dialog string nulls");
					exit(1);
				}
			}

			j++;
		}
	}
}

void writeItems(FILE *fd) {
	const char ***items = englishItems;
	const uint kNumInventoryItems = 92;

	for (uint i = 0; i < kNumInventoryItems; i++) {
		// Write item id
		writeByte(fd, i + 1);
		// Write animated flag
		if (i == 6 || i == 13 || i == 47 || i == 49 || i == 67 || i == 91)
			writeByte(fd, 0x01);
		else
			writeByte(fd, 0x00);

		// Write name and description (if exists) of an item
		uint j = 0;
		bool endItem = false;
		while (!endItem) {
			if (strcmp(items[i][j], "\n") == 0) { // Separator between name and description
				writeByte(fd, '\0');
			} else if (strcmp(items[i][j], "\n\n") == 0) {
				writeByte(fd, '\0');
				writeByte(fd, '\0');
				endItem = true;
			} else {
				if (fwrite(items[i][j], 1, strlen(items[i][j]), fd) != strlen(items[i][j])) {
					perror("Writing item string");
					exit(1);
				}
			}

			j++;
		}
	}
}

void writeSceneObjects(FILE* fd) {
	uint sceneObjTableAddrsPos = ftell(fd);
	uint16 sceneObjTableAddrs[42]{};
	uint16 curOffset = 0;
	for (uint i = 0; i < sceneObjects.size(); i++)
		writeUint16LE(fd, 0);

	curOffset += 84; // 2 bytes * 42 scenes

	for (uint i = 0; i < sceneObjects.size(); i++) {
		if (sceneObjects[i].size() > 0)
			sceneObjTableAddrs[i] = curOffset;

		uint firstObjsAddrFilePos = ftell(fd);
		Common::Array<uint16> sceneObjAddrs(sceneObjects[i].size(), 0);
		for (uint16 addr : sceneObjAddrs)
			writeUint16LE(fd, addr);

		curOffset += sizeof(uint16) * sceneObjAddrs.size();

		for (uint j = 0; j < sceneObjects[i].size(); j++) {
			sceneObjAddrs[j] = curOffset;

			// Write the object data
			sceneObjects[i][j].write(fd);
			curOffset += 19;

			// Name
			const char *name = englishSceneObjectNamesDescs[i][j].name;
			for (uint k = 0; k < strlen(name); k++) {
				if (name[k] == '\n')
					writeByte(fd, '\0');
				else
					writeByte(fd, name[k]);
			}
			writeByte(fd, '\0');
			curOffset += strlen(name) + 1;

			// Description (if exists)
			const char *description = englishSceneObjectNamesDescs[i][j].description;
			if (strlen(description) == 0) {
				writeByte(fd, '\0');
				writeByte(fd, '\0');
			} else if (strcmp(description, "\001") == 0) {
				writeByte(fd, '\001');
				writeByte(fd, '\0');
			} else {
				for (uint k = 0; k < strlen(description); k++) {
					if (description[k] == '\n')
						writeByte(fd, '\0');
					else
						writeByte(fd, description[k]);
				}
				writeByte(fd, '\0');
				writeByte(fd, '\0');
				curOffset += strlen(description);
			}
			curOffset += 2;
		}

		// Add zero addr to indicate the end of objects
		sceneObjAddrs.push_back(0);

		uint pos = ftell(fd);
		fseek(fd, firstObjsAddrFilePos, SEEK_SET);
		fwrite(sceneObjAddrs.data(), sizeof(uint16), sceneObjAddrs.size(), fd);
		fseek(fd, pos, SEEK_SET);
	}

	uint pos = ftell(fd);
	fseek(fd, sceneObjTableAddrsPos, SEEK_SET);
	for (uint i = 0; i < sceneObjects.size(); i++) {
		writeUint16LE(fd, sceneObjTableAddrs[i]);
	}
	fseek(fd, pos, SEEK_SET);
}

void writeMessages(FILE *fd) {
	// Write out message string block
	const char **messages = englishMessages;

	for (uint i = 0; i < kNumMessages; i++) {
		for (uint j = 0; j < strlen(messages[i]); j++) {
			if (messages[i][j] == '\n')
				writeByte(fd, '\0');
			else
				writeByte(fd, messages[i][j]);
		}
		writeByte(fd, '\0');
		writeByte(fd, '\0');
	}
}

void writeResource(FILE *fd, ResourceType resType) {
	uint currentFilePos = ftell(fd);
	uint prevFilePos = currentFilePos;
	uint32 resourceSize = 0;
	writeUint32LE(fd, resourceSize);

	switch (resType) {
	case kResCredits:
		writeCredits(fd);
		break;
	case kResDialogs:
		writeDialogs(fd);
		break;
	case kResItems:
		writeItems(fd);
		break;
	case kResSceneObjects:
		writeSceneObjects(fd);
		break;
	case kResMessages:
		writeMessages(fd);
		break;
	};

	currentFilePos = ftell(fd);
	resourceSize = currentFilePos - prevFilePos - sizeof(uint32);
	fseek(fd, prevFilePos, SEEK_SET);
	writeUint32LE(fd, resourceSize);
	fseek(fd, currentFilePos, SEEK_SET);
}

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

	// Skip messages block
	// It is written as a resource after the data segment
	uint msgBlockSize = 11415; // The size of messages block in the English exe
	fseek(fout, msgBlockSize, SEEK_CUR);

	if (fwrite(dsegEndBlock, DSEG_ENDBLK_SIZE, 1, fout) != 1) {
		perror("Writing data segment end block");
		exit(1);
	}

	for (uint i = 0; i < NUM_RESOURCES; i++) {
		writeResource(fout, ResourceType(i));
	}

	fclose(fout);

	return 0;
}
