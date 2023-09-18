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
struct CueLookupTable;

/**
 * A class able to load and parse Cue sheets
 */
class CueSheet {
public:
	CueSheet(const char *sheet);
	CueSheet(SeekableReadStream *stream);

public:
	enum CueFileType {
		kCueFileTypeBinary,
		kCueFileTypeAIFF,
		kCueFileTypeWave,
		kCueFileTypeMP3,
		kCueFileTypeMotorola,
	};

	enum CueTrackType {
		kCueFileTypeAudio,		// Audio (sector size: 2352)
		kCueFileTypeCDG,		// Karaoke CD+G (sector size: 2448)
		kCueFileTypeMode1_Raw,	// CD-ROM Mode 1 data (raw) (sector size: 2352), used by cdrdao
		kCueFileTypeMode1_2048,	// CD-ROM Mode 1 data (cooked) (sector size: 2048)
		kCueFileTypeMode1_2352,	// CD-ROM Mode 1 data (raw) (sector size: 2352)
		kCueFileTypeMode2_Raw,	// CD-ROM Mode 2 data (raw) (sector size: 2352), used by cdrdao
		kCueFileTypeMode2_2048,	// CD-ROM Mode 2 XA form-1 data (sector size: 2048)
		kCueFileTypeMode2_2324,	// CD-ROM Mode 2 XA form-2 data (sector size: 2324)
		kCueFileTypeMode2_2366,	// CD-ROM Mode 2 data (sector size: 2336)
		kCueFileTypeMode2_2352,	// CD-ROM Mode 2 data (raw) (sector size: 2352)
		kCueFileTypeCDI_2336,	// CDI Mode 2 data
		kCueFileTypeCDI_2352,	// CDI Mode 2 data
	};

	enum CueTrackFlags {
		kCueTrackFlag4ch  = 1 << 0, // Four channel audio
		kCueTrackFlagDCP  = 1 << 1, // Digital copy permitted
		kCueTrackFlagPre  = 1 << 2, // Pre-emphasis enabled, for audio tracks only
		kCueTrackFlagSCMS = 1 << 3, // Serial copy management system
	};

	struct CueTrack {
		int number = 0;
		CueTrackType type;
		String title;
		String performer;
		Array<int> indices;
		int pregap = 0;
		uint32 flags;
		int size = 2352;
	};

	struct CueFile {
		String name;
		CueFileType type = kCueFileTypeBinary;
		Array<CueTrack> tracks;
	};

	struct CueMetadata {
		String title;
		String date;
		String genre;
		String performer;
		String catalog;
	};

private:
	void parse(const char *sheet);

	int lookupInTable(CueLookupTable *table, const char *key);
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
	CueMetadata _metadata;

};

} // End of namespace Common

#endif
