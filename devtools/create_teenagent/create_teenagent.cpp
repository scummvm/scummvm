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

void writeStringsBlock(FILE *fd, const char **stringArr, uint size) {
	for (uint i = 0; i < size; i++) {
		for (uint j = 0; j < strlen(stringArr[i]); j++) {
			if (stringArr[i][j] == '\n')
				writeByte(fd, '\0');
			else
				writeByte(fd, stringArr[i][j]);
		}
		writeByte(fd, '\0');
		writeByte(fd, '\0');
	}
}

void writeCombinations(FILE *fd, Common::Language language) {
	const char **combineMessages = englishCombineMessages;
	if (language == CS_CZE)
		combineMessages = czechCombineMessages;
	else if (language == PL_POL)
		combineMessages = polishCombineMessages;
	else if (language == RU_RUS)
		combineMessages = russianCombineMessages;

	for (uint i = 0; i < kNumCombinations; i++) {
		combiningTable[i].write(fd);
		for (uint j = 0; j < strlen(combineMessages[i]); j++) {
			if (combineMessages[i][j] == '\n')
				writeByte(fd, '\0');
			else
				writeByte(fd, combineMessages[i][j]);
		}
		writeByte(fd, '\0');
		writeByte(fd, '\0');
	}
}

void writeDialogStacks(FILE *fd, Common::Language language) {
	const char ***dialogs = englishDialogs;
	if (language == CS_CZE)
		dialogs = czechDialogs;
	else if (language == PL_POL)
		dialogs = polishDialogs;
	else if (language == RU_RUS)
		dialogs = russianDialogs;

	uint16 offset = 0;
	uint16 dialogOffsets[kNumDialogs];

	for (uint i = 0; i < kNumDialogs; i++) {
		dialogOffsets[i] = offset;

		bool dialogEnd = false;
		uint j = 0;
		while (!dialogEnd) {
			offset += strlen(dialogs[i][j]);
			if (strcmp(dialogs[i][j], END_DIALOG) == 0)
				dialogEnd = true;
			j++;
		}
	}

	for (uint i = 0; i < sizeof(dialogStacks) / sizeof(uint16); i++) {
		if (dialogStacks[i] != 0xffff) {
			if (i == 0) {
				// skip ANIM_WAIT (0xff) byte
				writeUint16LE(fd, dialogOffsets[dialogStacks[i]] + 1);
			} else if (i == 190) {
				// There are two extra null bytes
				// in at the beginning of this dialog.
				// Skip them.
				writeUint16LE(fd, dialogOffsets[dialogStacks[i]] + 2);
			} else {
				writeUint16LE(fd, dialogOffsets[dialogStacks[i]]);
			}
		} else {
			writeUint16LE(fd, 0xffff);
		}
	}
}

void writeDialogs(FILE *fd, Common::Language language) {
	const char ***dialogs = englishDialogs;
	if (language == CS_CZE)
		dialogs = czechDialogs;
	else if (language == PL_POL)
		dialogs = polishDialogs;
	else if (language == RU_RUS)
		dialogs = russianDialogs;

	// Write out dialog string block
	static const char nulls[6] = "\0\0\0\0\0";
	for (uint i = 0; i < kNumDialogs; i++) {
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
				if (fwrite(dialogs[i][j], 1, strlen(dialogs[i][j]), fd) != strlen(dialogs[i][j])) {
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

void writeItems(FILE *fd, Common::Language language) {
	const char ***items = englishItems;
	if (language == CS_CZE)
		items = czechItems;
	else if (language == PL_POL)
		items = polishItems;
	else if (language == RU_RUS)
		items = russianItems;

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

void writeSceneObjects(FILE *fd, Common::Language language) {
	Common::Array<Common::Array<ObjectNameDesc>> *objNamesDescs = &englishSceneObjectNamesDescs;
	SettableObjectName *settableSceneObjects = englishSettableObjectNames;

	if (language == CS_CZE) {
		objNamesDescs = &czechSceneObjectNamesDescs;
		settableSceneObjects = czechSettableObjectNames;
	} else if (language == PL_POL) {
		objNamesDescs = &polishSceneObjectNamesDescs;
		settableSceneObjects = polishSettableObjectNames;
	} else if (language == RU_RUS) {
		objNamesDescs = &russianSceneObjectNamesDescs;
		settableSceneObjects = russianSettableObjectNames;
	}

	uint sceneObjTableAddrsPos = ftell(fd);
	uint16 sceneObjTableAddrs[42]{};
	uint16 curOffset = 0;
	for (uint i = 0; i < sceneObjects.size(); i++)
		writeUint16LE(fd, 0);

	curOffset += 84; // 2 bytes * 42 scenes

	for (uint i = 0; i < sceneObjects.size(); i++) {
		sceneObjTableAddrs[i] = curOffset;

		uint firstObjsAddrFilePos = ftell(fd);
		Common::Array<uint16> sceneObjAddrs(sceneObjects[i].size(), 0);
		// Add blank object to the end
		sceneObjAddrs.push_back(0);

		for (uint16 addr : sceneObjAddrs)
			writeUint16LE(fd, addr);

		curOffset += sizeof(uint16) * sceneObjAddrs.size();

		for (uint j = 0; j < sceneObjects[i].size(); j++) {
			sceneObjAddrs[j] = curOffset;

			// Write the object data
			sceneObjects[i][j].write(fd);
			curOffset += 19;

			// Name
			const char *name = (*objNamesDescs)[i][j]._name;
			for (uint k = 0; k < strlen(name); k++) {
				if (name[k] == '\n')
					writeByte(fd, '\0');
				else
					writeByte(fd, name[k]);
			}

			bool nameIsSettable = false;
			const char *setName = nullptr;
			for (byte k = 0; k < 4; k++) {
				if (strcmp(name, settableSceneObjects[k]._initialName) == 0) {
					nameIsSettable = true;
					setName = settableSceneObjects[k]._setName;

					if (strlen(setName) > strlen(settableSceneObjects[k]._initialName)) {
						uint nameLengthDiff = strlen(setName) - strlen(settableSceneObjects[k]._initialName);
						for (uint c = 0; c < nameLengthDiff; c++) {
							writeByte(fd, '\0');
							curOffset++;
						}
					}
					break;
				}
			}
			writeByte(fd, '\0');
			curOffset += strlen(name) + 1;

			// Description (if exists)
			const char *description = (*objNamesDescs)[i][j]._description;
			if (strlen(description) == 0) {
				writeByte(fd, '\0');
				writeByte(fd, '\0');
				curOffset += 2;
			} else if (strcmp(description, "\001") == 0) {
				writeByte(fd, '\001');
				curOffset++;
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
				curOffset += 2;
			}

			if (nameIsSettable) {
				fwrite(setName, 1, strlen(setName), fd);
				writeByte(fd, '\0');
				writeByte(fd, 0xFF);
				curOffset += strlen(setName) + 2;
			}
		}

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

void writeResource(FILE *fd, ResourceType resType, Common::Language language) {
	uint currentFilePos = ftell(fd);
	uint prevFilePos = currentFilePos;
	uint32 resourceSize = 0;
	writeUint32LE(fd, resourceSize);

	switch (resType) {
	case kResCredits: {
		const char **credits = englishCredits;
		if (language == CS_CZE)
			credits = czechCredits;
		else if (language == PL_POL)
			credits = polishCredits;
		else if (language == RU_RUS)
			credits = russianCredits;

		writeStringsBlock(fd, credits, kNumCredits);
		break;
	}
	case kResDialogStacks:
		writeDialogStacks(fd, language);
		break;
	case kResDialogs:
		writeDialogs(fd, language);
		break;
	case kResItems:
		writeItems(fd, language);
		break;
	case kResSceneObjects:
		writeSceneObjects(fd, language);
		break;
	case kResMessages: {
		const char **messages = englishMessages;
		if (language == CS_CZE)
			messages = czechMessages;
		else if (language == PL_POL)
			messages = polishMessages;
		else if (language == RU_RUS)
			messages = russianMessages;
		writeStringsBlock(fd, messages, kNumMessages);
		break;
	}
	case kResCombinations:
		writeCombinations(fd, language);
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

	// Write header
	fwrite("TEENAGENT", 9, 1, fout);
	writeByte(fout, TEENAGENT_DAT_VERSION);

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

	uint32 languageOffset = ftell(fout);

	for (uint lang = 0; lang < NUM_LANGS; lang++) {
		// Write language ID
		writeByte(fout, supportedLanguages[lang]);

		writeUint32LE(fout, 0);
	}
	writeByte(fout, (byte)Common::Language::UNK_LANG);

	for (uint lang = 0; lang < NUM_LANGS; lang++) {
		// Write offset to data
		uint32 dataOffset = ftell(fout);
		fseek(fout, languageOffset + (lang * 5 + 1), SEEK_SET);
		writeUint32LE(fout, dataOffset);

		fseek(fout, dataOffset, SEEK_SET);

		for (uint i = 0; i < NUM_RESOURCES; i++) {
			writeResource(fout, ResourceType(i), supportedLanguages[lang]);
		}
	}

	fclose(fout);

	return 0;
}
