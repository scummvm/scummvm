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

// Include messages
// This file is generated from the po files by the script po2c:
// tools/create_translations/po2c po/*.po > tools/create_translations/messages.h
#include "messages.h"

#define TRANSLATIONS_DAT_VER 1	// 1 byte

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

int stringSize(const char* string) {
	// Each string is preceded by its size coded on 2 bytes
	int len = strlen(string) + 1;
	return 2 + len;
	// The two lines below are an example if we want to align string writes
	// pad = DATAALIGNMENT - (len + 2) % DATAALIGNMENT;
	// return 2 + len + pad;
}

void writeString(FILE *fp, const char* string) {
	// Each string is preceded by its size coded on 2 bytes
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

int translationArraySize(const PoMessageEntry *msgs) {
	// ARRAYSIZE() macro does not work on _translations[index].msgs
	// We rely on the fact that the item of the array has an id of 1 instead.
	int size = 0;
	while (msgs[size].msgid != -1) {
		size++;
	}
	return size;
}

// Main
int main(int argc, char *argv[]) {
	FILE* outFile;
	int numLangs = ARRAYSIZE(_translations) - 1;
	int numMessages = ARRAYSIZE(_messageIds) - 1;
	int i, lang, nb;
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
	// We could write it at the start of each block but that would mean than
	// to go to block 4 we would have to go at the start of each preceding block,
	// read it size and skip it until we arrive at the block we want.
	// By having all the sizes at the start we just need to read the start of the
	// file and can then skip to the block we want.
	// Blocks are:
	//   1. List of languages with the language name
	//   2. Original messages (i.e. english)
	//   3. First translation
	//   4. Second translation
	//   ...
	
	// Write length for translation description
	// Each description
	len = 0;
	for (lang = 0; lang < numLangs; lang++) {
		len += stringSize(_translations[lang].lang);
		len += stringSize(_translations[lang].langname);
	}
	writeUint16BE(outFile, len);
	
	// Write size for the original language (english) block
	// It starts with the number of strings coded on 2 bytes followed by each
	// string (two bytes for the number of chars and the string itself).
	len = 2;
	for (i = 0; i < numMessages ; ++i)
		len += stringSize(_messageIds[i]);
	writeUint16BE(outFile, len);

	// Then comes the size of each translation block.
	// It starts with the number of strings coded on 2 bytes, the charset and then the strings.
	// For each string we have the string id (on two bytes) followed by
	// the string size (two bytes for the number of chars and the string itself).
	for (lang = 0; lang < numLangs; lang++) {
		len = 2 + stringSize(_translations[lang].charset);
		nb = translationArraySize(_translations[lang].msgs);
		for (i = 0; i < nb ; ++i)
			len += 2 + stringSize(_translations[lang].msgs[i].msgstr);
		writeUint16BE(outFile, len);
	}

	// Write list of languages
	for (lang = 0; lang < numLangs; lang++) {
		writeString(outFile, _translations[lang].lang);
		writeString(outFile, _translations[lang].langname);
	}
	
	// Write original messages
	writeUint16BE(outFile, numMessages);
	for (i = 0; i < numMessages ; ++i) {
		writeString(outFile, _messageIds[i]);
	}
	
	// Write translations
	for (lang = 0; lang < numLangs; lang++) {
		nb = translationArraySize(_translations[lang].msgs);
		writeUint16BE(outFile, nb);
		writeString(outFile, _translations[lang].charset);
		for (i = 0; i < nb ; ++i) {
			writeUint16BE(outFile, _translations[lang].msgs[i].msgid);
			writeString(outFile, _translations[lang].msgs[i].msgstr);
		}
	}

	fclose(outFile);

	return 0;
}
