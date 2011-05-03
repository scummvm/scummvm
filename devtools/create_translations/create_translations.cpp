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
 * This is a utility for create the translations.dat file from all the po files.
 * The generated files is used by ScummVM to propose translation of its GUI.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

 // HACK to allow building with the SDL backend on MinGW
// see bug #1800764 "TOOLS: MinGW tools building broken"
#ifdef main
#undef main
#endif // main

#include "create_translations.h"
#include "po_parser.h"

#define TRANSLATIONS_DAT_VER 2	// 1 byte

// Padding buffer (filled with 0) used if we want to aligned writes
// static uint8 padBuf[DATAALIGNMENT];

// Utility functions
// Some of the function are very simple but are factored out so that it would require
// minor modifications if we want for example to aligne writes on 4 bytes.
void writeByte(FILE *fp, uint8 b) {
	fwrite(&b, 1, 1, fp);
}

void writeUint16BE(FILE *fp, uint16 value) {
	writeByte(fp, (uint8)(value >> 8));
	writeByte(fp, (uint8)(value & 0xFF));
}

int stringSize(const char *string) {
	// Each string is preceded by its size coded on 2 bytes
	if (string == NULL)
		return 2;
	int len = strlen(string) + 1;
	return 2 + len;
	// The two lines below are an example if we want to align string writes
	// pad = DATAALIGNMENT - (len + 2) % DATAALIGNMENT;
	// return 2 + len + pad;
}

void writeString(FILE *fp, const char *string) {
	// Each string is preceded by its size coded on 2 bytes
	if (string == NULL) {
		writeUint16BE(fp, 0);
		return;
	}
	int len = strlen(string) + 1;
	writeUint16BE(fp, len);
	fwrite(string, len, 1, fp);
	// The commented lines below are an example if we want to align string writes
	// It replaces the two lines above.
	// int pad = DATAALIGNMENT - (len + 2) % DATAALIGNMENT;
	// writeUint16BE(fp, len + pad);
	// fwrite(string, len, 1, fp);
	// fwrite(padBuf, pad, 1, fp);
}

// Main
int main(int argc, char *argv[]) {
	// Build the translation list
	PoMessageList messageIds;
	PoMessageEntryList **translations = new PoMessageEntryList*[argc - 1];
	int numLangs = 0;
	for (int i = 1; i < argc; ++i) {
		translations[numLangs] = parsePoFile(argv[i], messageIds);
		if (translations[numLangs] != NULL)
			++numLangs;
	}

	FILE *outFile;
	int i, lang;
	int len;

	// Padding buffer initialization (filled with 0)
	// used if we want to aligned writes
	// for (i = 0; i < DATAALIGNMENT; i++)
	//	padBuf[i] = 0;

	outFile = fopen("translations.dat", "wb");

	// Write header
	fwrite("TRANSLATIONS", 12, 1, outFile);

	writeByte(outFile, TRANSLATIONS_DAT_VER);

	// Write number of translations
	writeUint16BE(outFile, numLangs);

	// Write the length of each data block here.
	// We could write it at the start of each block but that would mean that
	// to go to block 4 we would have to go at the start of each preceding block,
	// read its size and skip it until we arrive at the block we want.
	// By having all the sizes at the start we just need to read the start of the
	// file and can then skip to the block we want.
	// Blocks are:
	//   1. List of languages with the language name
	//   2. Original messages (i.e. english)
	//   3. First translation
	//   4. Second translation
	//   ...

	// Write length for translation description
	len = 0;
	for (lang = 0; lang < numLangs; lang++) {
		len += stringSize(translations[lang]->language());
		len += stringSize(translations[lang]->languageName());
	}
	writeUint16BE(outFile, len);

	// Write size for the original language (english) block
	// It starts with the number of strings coded on 2 bytes followed by each
	// string (two bytes for the number of chars and the string itself).
	len = 2;
	for (i = 0; i < messageIds.size(); ++i)
		len += stringSize(messageIds[i]);
	writeUint16BE(outFile, len);

	// Then comes the size of each translation block.
	// It starts with the number of strings coded on 2 bytes, the charset and then the strings.
	// For each string we have the string id (on two bytes) followed by
	// the string size (two bytes for the number of chars and the string itself).
	for (lang = 0; lang < numLangs; lang++) {
		len = 2 + stringSize(translations[lang]->charset());
		for (i = 0; i < translations[lang]->size(); ++i) {
			len += 2 + stringSize(translations[lang]->entry(i)->msgstr);
			len += stringSize(translations[lang]->entry(i)->msgctxt);
		}
		writeUint16BE(outFile, len);
	}

	// Write list of languages
	for (lang = 0; lang < numLangs; lang++) {
		writeString(outFile, translations[lang]->language());
		writeString(outFile, translations[lang]->languageName());
	}

	// Write original messages
	writeUint16BE(outFile, messageIds.size());
	for (i = 0; i < messageIds.size(); ++i) {
		writeString(outFile, messageIds[i]);
	}

	// Write translations
	for (lang = 0; lang < numLangs; lang++) {
		writeUint16BE(outFile, translations[lang]->size());
		writeString(outFile, translations[lang]->charset());
		for (i = 0; i < translations[lang]->size(); ++i) {
			writeUint16BE(outFile, messageIds.findIndex(translations[lang]->entry(i)->msgid));
			writeString(outFile, translations[lang]->entry(i)->msgstr);
			writeString(outFile, translations[lang]->entry(i)->msgctxt);
		}
	}

	fclose(outFile);

	// Clean the memory
	for (i = 0; i < numLangs; ++i)
		delete translations[i];
	delete[] translations;

	return 0;
}
