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

#ifndef COMMON_FORMATS_CUE_H
#define COMMON_FORMATS_CUE_H

#include "common/array.h"

namespace Common {

class SeekableReadStream;

/**
 * A class able to load and parse Cue sheets
 */
class CueSheet {
public:
	CueSheet(const char *sheet);
	CueSheet(SeekableReadStream *stream);

public:
	enum FileType {
		kFileTypeBinary,
		kFileTypeAIFF,
		kFileTypeWave,
		kFileTypeMP3,
		kFileTypeMotorola,
	};

	enum TrackType {
		kTrackTypeAudio,		// Audio (sector size: 2352)
		kTrackTypeCDG,			// Karaoke CD+G (sector size: 2448)
		kTrackTypeMode1_Raw,	// CD-ROM Mode 1 data (raw) (sector size: 2352), used by cdrdao
		kTrackTypeMode1_2048,	// CD-ROM Mode 1 data (cooked) (sector size: 2048)
		kTrackTypeMode1_2352,	// CD-ROM Mode 1 data (raw) (sector size: 2352)
		kTrackTypeMode2_Raw,	// CD-ROM Mode 2 data (raw) (sector size: 2352), used by cdrdao
		kTrackTypeMode2_2048,	// CD-ROM Mode 2 XA form-1 data (sector size: 2048)
		kTrackTypeMode2_2324,	// CD-ROM Mode 2 XA form-2 data (sector size: 2324)
		kTrackTypeMode2_2366,	// CD-ROM Mode 2 data (sector size: 2336)
		kTrackTypeMode2_2352,	// CD-ROM Mode 2 data (raw) (sector size: 2352)
		kTrackTypeCDI_2336,		// CDI Mode 2 data
		kTrackTypeCDI_2352,		// CDI Mode 2 data
	};

	enum TrackFlags {
		kTrackFlag4ch  = 1 << 0, // Four channel audio
		kTrackFlagDCP  = 1 << 1, // Digital copy permitted
		kTrackFlagPre  = 1 << 2, // Pre-emphasis enabled, for audio tracks only
		kTrackFlagSCMS = 1 << 3, // Serial copy management system
	};

	struct CueFile {
		String name;
		FileType type = kFileTypeBinary;
	};

	struct CueTrack {
		int number = 0;
		CueFile file;
		TrackType type;
		String title;
		String performer;
		Array<int> indices;
		int pregap = 0;
		uint32 flags;
		int size = 2352;
	};

	struct CueMetadata {
		String title;
		String date;
		String genre;
		String performer;
		String catalog;
	};

	struct LookupTable;

	Array<CueFile> files();
	Array<CueTrack> tracks();
	CueTrack *getTrack(int tracknum);
	CueTrack *getTrackAtFrame(int frame);

private:
	void parse(const char *sheet);

	int lookupInTable(const LookupTable *table, const char *key);
	int parseMSF(const char *str);

	void parseHeaderContext(const char *line);
	void parseFilesContext(const char *line);
	void parseTracksContext(const char *line);

private:
	int _lineNum = 0;
	int _context;
	int _currentFile = -1;
	int _currentTrack = -1;

	Array<CueFile> _files;
	Array<CueTrack> _tracks;
	CueMetadata _metadata;

};

} // End of namespace Common

#endif
