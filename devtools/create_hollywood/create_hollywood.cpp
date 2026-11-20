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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <stdio.h>
#include <string.h>

#include "../create_translations/po_parser.h"

static void writeUint16LE(FILE *file, size_t value) {
	fputc(value & 0xff, file);
	fputc((value >> 8) & 0xff, file);
}

static bool writeCatalog(const char *fileName, const PoMessageEntryList &catalog) {
	if (!catalog.useUTF8() || !catalog.size() || catalog.size() > 0xffff) {
		fprintf(stderr, "Expected a nonempty UTF-8 catalog with at most 65535 entries\n");
		return false;
	}
	PoMessageList keys;
	for (int i = 0; i < catalog.size(); ++i) {
		const PoMessageEntry *entry = catalog.entry(i);
		if (!entry->msgctxt || strlen(entry->msgctxt) > 0xffff || strlen(entry->msgstr) > 0xffff || keys.findIndex(entry->msgctxt) >= 0) {
			fprintf(stderr, "Invalid Runaway text entry: %s\n", entry->msgid);
			return false;
		}
		keys.insert(entry->msgctxt);
	}

	FILE *file = fopen(fileName, "wb");
	if (!file) {
		fprintf(stderr, "Cannot create %s\n", fileName);
		return false;
	}
	// Version, entry count, then length-prefixed UTF-8 key/value pairs.
	fwrite("RWTX", 4, 1, file);
	writeUint16LE(file, 2);
	writeUint16LE(file, catalog.size());
	for (int i = 0; i < catalog.size(); ++i) {
		const PoMessageEntry *entry = catalog.entry(i);
		const size_t keySize = strlen(entry->msgctxt);
		const size_t textSize = strlen(entry->msgstr);
		writeUint16LE(file, keySize);
		writeUint16LE(file, textSize);
		fwrite(entry->msgctxt, 1, keySize, file);
		fwrite(entry->msgstr, 1, textSize, file);
	}
	const bool success = !ferror(file);
	const int closeResult = fclose(file);
	return success && closeResult == 0;
}

int main(int argc, char *argv[]) {
	if (argc != 3) {
		fprintf(stderr, "Usage: create_hollywood OUTPUT INPUT.po\n");
		return 1;
	}
	PoMessageList messages;
	PoMessageEntryList *catalog = parsePoFile(argv[2], messages);
	if (!catalog) {
		fprintf(stderr, "Cannot open %s\n", argv[2]);
		return 1;
	}
	const bool success = writeCatalog(argv[1], *catalog);
	delete catalog;
	return success ? 0 : 1;
}
