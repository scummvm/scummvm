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

#include "common/str.h"
#include "common/formats/cue.h"

namespace Common {

enum {
	kCueContextHeader,
	kCueContextFiles,
	kCueContextTracks,
};

static String nexttok(const char *s, const char **newP = nullptr) {
	String res;

	// Scan first non-whitespace
	while (*s && (*s == ' ' || *s == '\t')) // If we see a whitespace
		s++;

	if (*s == '"') { // If it is a string then scan till end quote
		s++; // skip the quote

		while (*s && *s != '"')
			res += *s++;

		if (*s == '"')
			s++;
	} else {
		while (*s && *s != ' ' && *s != '\t' && *s != '\n' && *s != '\r')
			res += *s++;
	}

	if (newP)
		*newP = s;

	return res;
}


CueSheet::CueSheet(const char *sheet) {
	String line;
	_context = kCueContextHeader;
	const char *s = sheet;
	int lineNum = 0;

	while (*s) {
		line.clear();

		// Get a line
		while (*s && *s != '\n' && *s != '\r') // If we see a whitespace
			line += *s++;

		// Skip any newlines and line feeds
		while (*s == '\n' || *s == '\r') {
			if (*s == '\n')
				lineNum++;

			s++;
		}


		if (line.empty())
			continue;

		String firstToken = nexttok(line.c_str()); // Peek into comments

		// Skip non-conformant comments
		if (firstToken[0] == ';' || (firstToken.size() > 1 && firstToken[0] == '/' && firstToken[1] == '/'))
			continue;

		switch (_context) {
		case kCueContextHeader:
			parseHeaderContext(line.c_str());
			break;

		case kCueContextFiles:
			parseFilesContext(line.c_str());
			break;

		case kCueContextTracks:
			parseTracksContext(line.c_str());
			break;

		default:
			error("CueSheet: bad context %d at line %d", _context, lineNum);
		}
	}
}

struct LookupTable {
	const char *key;
	int value;
};

static int lookupInTable(LookupTable *table, const char *key) {
	while (table->key) {
		if (!strcmp(key, table->key))
			return table->value;

		table++;
	}

	error("CueSheet::lookupInTable(): Unknown token %s", key);
}

LookupTable fileTypes[] = {
	{ "BINARY",   CueSheet::kCueFileTypeBinary },
	{ "AIFF",     CueSheet::kCueFileTypeAIFF },
	{ "WAVE",     CueSheet::kCueFileTypeWave },
	{ "MP3",      CueSheet::kCueFileTypeMP3 },
	{ "MOTOROLA", CueSheet::kCueFileTypeMotorola },
	{ 0, 0 }
};

void CueSheet::parseHeaderContext(const char *line) {
	const char *s = line;

	String command = nexttok(s, &s);

	if (command == "FILE") {
		_files.push_back(CueFile());

		_currentFile++;

		_files[_currentFile].name = nexttok(s, &s);

		String type = nexttok(s, &s);

		_files[_currentFile].type = kCueFileTypeBinary;
		_files[_currentFile].type = (CueFileType)lookupInTable(fileTypes, type.c_str());

		_context = kCueContextFiles;
	}
}

void CueSheet::parseFilesContext(const char *line) {
}

void CueSheet::parseTracksContext(const char *line) {
}

} // End of namespace Common
