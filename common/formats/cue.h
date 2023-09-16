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

public:
	enum CueFileType {
		kCueFileTypeBinary,
		kCueFileTypeAIFF,
		kCueFileTypeWave,
		kCueFileTypeMP3,
		kCueFileTypeMotorola,
	};

	struct CueFile {
		String name;
		CueFileType type = kCueFileTypeBinary;
	};

private:
	void parseHeaderContext(const char *line);
	void parseFilesContext(const char *line);
	void parseTracksContext(const char *line);

private:
	int _context;
	int _currentFile = -1;

	Common::Array<CueFile> _files;
};

} // End of namespace Common

#endif
