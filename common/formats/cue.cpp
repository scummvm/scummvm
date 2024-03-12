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
#include "common/stream.h"
#include "common/formats/cue.h"

namespace Common {

enum {
	kContextHeader,
	kContextFiles,
	kContextTracks,
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
	parse(sheet);
}

CueSheet::CueSheet(SeekableReadStream *stream) {
	int size = stream->size();
	char *data = (char *)calloc(size + 1, 1); // null-terminated string

	stream->read(data, size);

	parse(data);

	free(data);
}

void CueSheet::parse(const char *sheet) {
	String line;
	_context = kContextHeader;
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
		case kContextHeader:
			parseHeaderContext(line.c_str());
			break;

		case kContextFiles:
			parseFilesContext(line.c_str());
			break;

		case kContextTracks:
			parseTracksContext(line.c_str());
			break;

		default:
			error("CueSheet: bad context %d at line %d", _context, _lineNum);
		}
	}
}

struct CueSheet::LookupTable {
	const char *key;
	int value;
};

int CueSheet::lookupInTable(const LookupTable *table, const char *key) {
	while (table->key) {
		if (!strcmp(key, table->key))
			return table->value;

		table++;
	}

	error("CueSheet::lookupInTable(): Unknown lookup token %s at line %d", key, _lineNum);
}

int CueSheet::parseMSF(const char *str) {
	int min = 0, sec = 0, frm = 0;

	if (sscanf(str, "%d:%d:%d", &min, &sec, &frm) != 3)
		warning("Malformed MSF at line %d: %s", _lineNum, str);

	return frm + 75 * (sec + 60 * min);
}

static const CueSheet::LookupTable fileTypes[] = {
	{ "BINARY",   CueSheet::kFileTypeBinary },
	{ "AIFF",     CueSheet::kFileTypeAIFF },
	{ "WAVE",     CueSheet::kFileTypeWave },
	{ "MP3",      CueSheet::kFileTypeMP3 },
	{ "MOTOROLA", CueSheet::kFileTypeMotorola },
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

		_files[_currentFile].type = (FileType)lookupInTable(fileTypes, type.c_str());

		_context = kContextFiles;

		_currentTrack = -1;

		debug(5, "File: %s, type: %s (%d)", _files[_currentFile].name.c_str(), type.c_str(), _files[_currentFile].type);
	} else if (command == "TITLE") {
		_metadata.title = nexttok(s, &s);

		debug(5, "Title: %s", _metadata.title.c_str());
	} else if (command == "PERFORMER") {
		_metadata.performer = nexttok(s, &s);

		debug(5, "Performer: %s", _metadata.performer.c_str());
	} else if (command == "CATALOG") {
		_metadata.catalog = nexttok(s, &s);

		debug(5, "Catalog: %s", _metadata.catalog.c_str());
	} else if (command == "REM") {
		String subcommand = nexttok(s, &s);

		if (subcommand == "DATE") {
			_metadata.date = nexttok(s, &s);
			debug(5, "Date: %s", _metadata.date.c_str());
		} else if (subcommand == "GENRE") {
			_metadata.genre = nexttok(s, &s);
			debug(5, "Genre: %s", _metadata.genre.c_str());
		} else if (subcommand == "COMMENT") {
			debug(5, "Skipping Comment: %s", s);
		} else {
			warning("CueSheet: Unprocessed REM subcommand %s at line %d", subcommand.c_str(), _lineNum);
		}
	} else {
		warning("CueSheet: Unprocessed command %s at line %d", command.c_str(), _lineNum);
	}
}

static const CueSheet::LookupTable trackTypes[] = {
	{ "AUDIO",      CueSheet::kTrackTypeAudio },		// Audio (sector size: 2352)
	{ "CDG",        CueSheet::kTrackTypeCDG },			// Karaoke CD+G (sector size: 2448)
	{ "MODE1_RAW",  CueSheet::kTrackTypeMode1_Raw },	// CD-ROM Mode 1 data (raw) (sector size: 2352), used by cdrdao
	{ "MODE1/2048", CueSheet::kTrackTypeMode1_2048 },	// CD-ROM Mode 1 data (cooked) (sector size: 2048)
	{ "MODE1/2352", CueSheet::kTrackTypeMode1_2352 },	// CD-ROM Mode 1 data (raw) (sector size: 2352)
	{ "MODE2_RAW",  CueSheet::kTrackTypeMode2_Raw },	// CD-ROM Mode 2 data (raw) (sector size: 2352), used by cdrdao
	{ "MODE2/2048", CueSheet::kTrackTypeMode2_2048 },	// CD-ROM Mode 2 XA form-1 data (sector size: 2048)
	{ "MODE2/2324", CueSheet::kTrackTypeMode2_2324 },	// CD-ROM Mode 2 XA form-2 data (sector size: 2324)
	{ "MODE2/2336", CueSheet::kTrackTypeMode2_2366 },	// CD-ROM Mode 2 data (sector size: 2336)
	{ "MODE2/2352", CueSheet::kTrackTypeMode2_2352 },	// CD-ROM Mode 2 data (raw) (sector size: 2352)
	{ "CDI/2336",   CueSheet::kTrackTypeCDI_2336 },		// CDI Mode 2 data
	{ "CDI/2352",   CueSheet::kTrackTypeCDI_2352 },		// CDI Mode 2 data
	{ 0, 0 }
};

static const CueSheet::LookupTable trackTypesSectorSizes[] = {
	{ "AUDIO",      2352 },
	{ "CDG",        2448 },
	{ "MODE1_RAW",  2352 },
	{ "MODE1/2048", 2048 },
	{ "MODE1/2352", 2352 },
	{ "MODE2_RAW",  2352 },
	{ "MODE2/2048", 2048 },
	{ "MODE2/2324", 2324 },
	{ "MODE2/2336", 2336 },
	{ "MODE2/2352", 2352 },
	{ "CDI/2336",   2336 },
	{ "CDI/2352",   2352 },
	{ 0, 0 }
};

void CueSheet::parseFilesContext(const char *line) {
	const char *s = line;

	String command = nexttok(s, &s);

	if (command == "TRACK") {
		int trackNum = atoi(nexttok(s, &s).c_str());
		String trackType = nexttok(s, &s);

		// We have to add + 2 here because _currentTrack is a counter
		// for the array, which is 0-indexed, while the disc's track
		// numbers are 1-indexed. The next track, in disc numbering,
		// will be two greater than _currentTrack.
		if (trackNum < 0 || (_currentTrack > 0 && _currentTrack + 2 != trackNum)) {
			warning("CueSheet: Incorrect track number. Expected %d but got %d at line %d", _currentTrack + 1, trackNum, _lineNum);
		} else {
			_tracks.push_back(CueTrack());

			// Array is 0-indexed, track numbers are 1-indexed
			_currentTrack = trackNum - 1;
			_tracks[_currentTrack].number = trackNum;
			_tracks[_currentTrack].type = (TrackType)lookupInTable(trackTypes, trackType.c_str());
			_tracks[_currentTrack].size = lookupInTable(trackTypesSectorSizes, trackType.c_str());
			_tracks[_currentTrack].file = _files[_currentFile];

			debug(5, "Track: %d type: %s (%d)", trackNum, trackType.c_str(), _tracks[_currentTrack].type);
		}

		_context = kContextTracks;
	} else {
		warning("CueSheet: Unprocessed file command %s at line %d", command.c_str(), _lineNum);
	}

}

static const CueSheet::LookupTable trackFlags[] = {
	{ "4CH",  CueSheet::kTrackFlag4ch  },
	{ "DCP",  CueSheet::kTrackFlagDCP  },
	{ "PRE",  CueSheet::kTrackFlagPre  },
	{ "SCMS", CueSheet::kTrackFlagSCMS },
	{ 0, 0 }
};

void CueSheet::parseTracksContext(const char *line) {
	const char *s = line;

	String command = nexttok(s, &s);

	if (command == "TRACK") {
		parseFilesContext(line);
	} else if (command == "TITLE") {
		_tracks[_currentTrack].title = nexttok(s, &s);

		debug(5, "Track title: %s", _tracks[_currentTrack].title.c_str());
	} else if (command == "INDEX") {
		int indexNum = atoi(nexttok(s, &s).c_str());
		int frames = parseMSF(nexttok(s, &s).c_str());

		for (int i = (int)_tracks[_currentTrack].indices.size(); i <= indexNum; i++) {
			// -1 indicates "no index" to let callers guard against
			// interpreting these as real values
			_tracks[_currentTrack].indices.push_back(-1);
		}

		_tracks[_currentTrack].indices[indexNum] = frames;

		debug(5, "Index: %d, frames: %d", indexNum, frames);
	} else if (command == "PREGAP") {
		_tracks[_currentTrack].pregap = parseMSF(nexttok(s, &s).c_str());

		debug(5, "Track pregap: %d", _tracks[_currentTrack].pregap);
	} else if (command == "FLAGS") {
		String flag;
		uint32 flags = 0;

		while (*s) {
			flag = nexttok(s, &s);

			flags |= lookupInTable(trackFlags, flag.c_str());
		}

		_tracks[_currentTrack].flags = flags;

		debug(5, "Track flags: %d", _tracks[_currentTrack].flags);
	} else if (command == "FILE") {
		parseHeaderContext(line);
	} else if (command == "PERFORMER") {
		_tracks[_currentTrack].performer = nexttok(s, &s);

		debug(5, "Track performer: %s", _tracks[_currentTrack].performer.c_str());
	} else {
		warning("CueSheet: Unprocessed track command %s at line %d", command.c_str(), _lineNum);
	}
}

Array<CueSheet::CueFile> CueSheet::files() {
	return _files;
}

Array<CueSheet::CueTrack> CueSheet::tracks() {
	return _tracks;
}

CueSheet::CueTrack *CueSheet::getTrack(int tracknum) {
	for (uint i = 0; i < _tracks.size(); i++) {
		if (_tracks[i].number == tracknum) {
			return &_tracks[i];
		}
	}

	return nullptr;
}

CueSheet::CueTrack *CueSheet::getTrackAtFrame(int frame) {
	for (uint i = 0; i < _tracks.size(); i++) {
		// Inside pregap
		if (_tracks[i].indices[0] >= 0 && frame >= _tracks[i].indices[0] && frame < _tracks[i].indices.back()) {
			debug(5, "CueSheet::getTrackAtFrame: Returning track %i (pregap)", _tracks[i].number);
			return &_tracks[i];
		}

		// Between index 1 and the start of the subsequent track.
		// Index 0 of the next track is the start of its pregap.
		// For tracks which have pregaps, we want to use that as the
		// frame to determine the edge of the track; otherwise, we'll
		// need to use the start of index 1, eg the start of the track.
		if (i+1 < _tracks.size()) {
			int nextIndex;
			CueSheet::CueTrack nextTrack = _tracks[i+1];
			// If there's more than one index, *and* index 0 looks
			// nullish, use index 1
			if (nextTrack.indices.size() > 1 && nextTrack.indices[0] == -1) {
				nextIndex = nextTrack.indices[1];
			// Otherwise, use index 0
			} else {
				nextIndex = nextTrack.indices[0];
			}

			 if (frame >= _tracks[i].indices.back() && frame < nextIndex) {
				debug(5, "CueSheet::getTrackAtFrame: Returning track %i (inside content)", _tracks[i].number);
				return &_tracks[i];
			}
		}
	}

	// Not found within any tracks, but could be in the final track.
	// Note: this looks weird, but is correct (or correct-ish).
	// The cuesheet tracks the *starting* index of a song, but not
	// the *duration* of a track. Without having access to the raw
	// disc data, we don't actually know how long this track is.
	// As a result, if we see any frame that comes after the
	// start of the final track, we need to assume it's
	// a part of that track.
	if (frame > _tracks.back().indices.back()) {
		debug(5, "CueSheet::getTrackAtFrame: Returning final track");
		return &_tracks.back();
	}

	// Still not found; could indicate a gap between indices
	warning("CueSheet::getTrackAtFrame: Not returning a track; does the cuesheet have a gap between indices?");
	return nullptr;
}

} // End of namespace Common
