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

#include "common/debug.h"
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

	while (*s) {
		line.clear();

		// Get a line
		while (*s && *s != '\n' && *s != '\r') // If we see a whitespace
			line += *s++;

		// Skip any newlines and line feeds
		while (*s == '\n' || *s == '\r') {
			if (*s == '\n')
				_lineNum++;

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
			error("CueSheet: bad context %d at line %d", _context, _lineNum);
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

int CueSheet::parseMSF(const char *str) {
	int min = 0, sec = 0, frm = 0;

	if (sscanf(str, "%d:%d:%d", &min, &sec, &frm) != 3)
		warning("Malformed MSF at line %d: %s", _lineNum, str);

	return frm + 75 * sec + 75 * 60 * min;
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

		_currentTrack = -1;

		debug(5, "File: %s, type: %s (%d)", _files[_currentFile].name.c_str(), type.c_str(), _files[_currentFile].type);
	} else if (command == "TITLE") {
		_metadata.title = nexttok(s, &s);

		debug(5, "Title: %s", _metadata.title.c_str());
	} else if (command == "PERFORMER") {
		_metadata.performer = nexttok(s, &s);

		debug(5, "Performer: %s", _metadata.performer.c_str());
	} else if (command == "REM") {
		String subcommand = nexttok(s, &s);

		if (subcommand == "DATE") {
			_metadata.date = nexttok(s, &s);
			debug(5, "Date: %s", _metadata.date.c_str());
		} else if (subcommand == "GENRE") {
			_metadata.genre = nexttok(s, &s);
			debug(5, "Genre: %s", _metadata.genre.c_str());
		} else {
			warning("CueSheet: Unprocessed REM subcommand %s", subcommand.c_str());
		}
	} else {
		warning("CueSheet: Unprocessed command %s", command.c_str());
	}
}

LookupTable trackTypes[] = {
	{ "AUDIO",      CueSheet::kCueFileTypeAudio }, // Audio (sector size: 2352)
	{ "CDG",        CueSheet::kCueFileTypeCDG }, // Karaoke CD+G (sector size: 2448)
	{ "MODE1_RAW",  CueSheet::kCueFileTypeMode1_Raw }, // CD-ROM Mode 1 data (raw) (sector size: 2352), used by cdrdao
	{ "MODE1/2048", CueSheet::kCueFileTypeMode1_2048 }, // CD-ROM Mode 1 data (cooked) (sector size: 2048)
	{ "MODE1/2352", CueSheet::kCueFileTypeMode1_2352 }, // CD-ROM Mode 1 data (raw) (sector size: 2352)
	{ "MODE2_RAW",  CueSheet::kCueFileTypeMode2_Raw }, // CD-ROM Mode 2 data (raw) (sector size: 2352), used by cdrdao
	{ "MODE2/2048", CueSheet::kCueFileTypeMode2_2048 }, // CD-ROM Mode 2 XA form-1 data (sector size: 2048)
	{ "MODE2/2324", CueSheet::kCueFileTypeMode2_2324 }, // CD-ROM Mode 2 XA form-2 data (sector size: 2324)
	{ "MODE2/2336", CueSheet::kCueFileTypeMode2_2366 }, // CD-ROM Mode 2 data (sector size: 2336)
	{ "MODE2/2352", CueSheet::kCueFileTypeMode2_2352 }, // CD-ROM Mode 2 data (raw) (sector size: 2352)
	{ "CDI/2336",   CueSheet::kCueFileTypeCDI_2336 }, // CDI Mode 2 data
	{ "CDI/2352",   CueSheet::kCueFileTypeCDI_2352 }, // CDI Mode 2 data
};

void CueSheet::parseFilesContext(const char *line) {
	const char *s = line;

	String command = nexttok(s, &s);

	if (command == "TRACK") {
		int trackNum = atoi(nexttok(s, &s).c_str());
		String trackType = nexttok(s, &s);

		if (trackNum < 0 || (_currentTrack > 0 && _currentTrack + 1 != trackNum)) {
			warning("CueSheet: Incorrect track number. Expected %d but got %d", _currentTrack + 1, trackNum);
		} else {
			for (int i = _files[_currentFile].tracks.size(); i <= trackNum; i++)
				_files[_currentFile].tracks.push_back(CueTrack());

			_currentTrack = trackNum;
			_files[_currentFile].tracks[_currentTrack].type = (CueTrackType)lookupInTable(trackTypes, trackType.c_str());

			debug(5, "Track: %d type: %s (%d)", trackNum, trackType.c_str(), _files[_currentFile].tracks[_currentTrack].type);
		}

		_context = kCueContextTracks;
	} else {
		warning("CueSheet: Unprocessed file command %s", command.c_str());
	}

}

void CueSheet::parseTracksContext(const char *line) {
	const char *s = line;

	String command = nexttok(s, &s);

	if (command == "TRACK") {
		parseFilesContext(line);
	} else if (command == "TITLE") {
		_files[_currentFile].tracks[_currentTrack].title = nexttok(s, &s);

		debug(5, "Track title: %s", _files[_currentFile].tracks[_currentTrack].title.c_str());
	} else if (command == "INDEX") {
		int indexNum = atoi(nexttok(s, &s).c_str());
		int frames = parseMSF(nexttok(s, &s).c_str());

		for (int i = _files[_currentFile].tracks[_currentTrack].indices.size(); i <= indexNum; i++)
			_files[_currentFile].tracks[_currentTrack].indices.push_back(0);

		_files[_currentFile].tracks[_currentTrack].indices[indexNum] = frames;

		debug(5, "Index: %d, frames: %d", indexNum, frames);
	} else if (command == "FILE") {
		parseHeaderContext(line);
	} else {
		warning("CueSheet: Unprocessed track command %s", command.c_str());
	}
}

} // End of namespace Common
